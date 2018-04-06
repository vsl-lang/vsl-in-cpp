#include "ast/opKind.hpp"
#include "irgen/passes/irEmitter/irEmitter.hpp"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Casting.h"
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>

IREmitter::IREmitter(VSLContext& vslCtx, Diag& diag, FuncScope& func,
    GlobalScope& global, TypeConverter& converter, llvm::Module& module)
    : vslCtx{ vslCtx }, diag{ diag }, func{ func }, global{ global },
    converter{ converter }, module{ module }, llvmCtx{ module.getContext() },
    builder{ llvmCtx }, allocaInsertPoint{ nullptr }, vslCtor{ nullptr },
    vslDtor{ nullptr }
{
}

void IREmitter::visitFunction(FunctionNode& node)
{
    // setup parameters/scope and stuff
    Value funcVal = global.get(node.getName());
    setupFuncBody(node, funcVal);
    func.setReturnType(node.getReturnType());
    // generate the function body
    genFuncBody(node);
    // cleanup function
    cleanupFuncBody(node);
}

void IREmitter::visitExtFunc(ExtFuncNode& node)
{
    assert(func.empty() && "parser didn't reject extfunc within func");
}

void IREmitter::visitParam(ParamNode& node)
{
}

void IREmitter::visitVariable(VariableNode& node)
{
    // make sure the variable type is valid
    if (!node.getType()->isValid())
    {
        diag.print<Diag::INVALID_VAR_TYPE>(node);
        return;
    }
    llvm::Type* llvmType = converter.convert(node.getType());
    llvm::Value* llvmValue;
    if (isGlobal())
    {
        // global variable
        // declare the global variable
        llvm::GlobalVariable* var = genGlobalVar(node.getAccess(),
            node.getType(), llvmType, node.getName());
        if (!var)
        {
            diag.print<Diag::VAR_ALREADY_DEFINED>(node);
            return;
        }
        llvmValue = var;
        // declare the constructor function
        llvm::Function* ctor = genGlobalVarCtor(var);
        // generate the destructor function
        genGlobalVarDtor(var, node.getType());
        // start inserting at the constructor
        builder.SetInsertPoint(&ctor->back());
    }
    else
    {
        // local variable
        llvm::AllocaInst* inst = createEntryAlloca(llvmType, node.getName());
        llvmValue = inst;
        // add to current scope
        if (func.set(node.getName(), Value::getVar(node.getType(), inst)))
        {
            diag.print<Diag::VAR_ALREADY_DEFINED>(node);
            inst->eraseFromParent();
            return;
        }
    }
    // generate initialization code
    node.getInit().accept(*this);
    Value init = copyValue(result);
    result = Value::getNull();
    // before initializing the variable, make sure that the initializer
    //  expression is actually valid
    bool valid = true;
    if (!init)
    {
        valid = false;
    }
    // match the var and init types
    else if (node.getType() != init.getVSLType())
    {
        diag.print<Diag::MISMATCHING_VAR_TYPES>(node, *init.getVSLType());
        valid = false;
    }
    // store the variable
    if (valid)
    {
        storeValue(init, Value::getVar(node.getType(), llvmValue));
    }
    if (isGlobal())
    {
        // this was inserting inside a constructor function so it must be
        //  terminated with a return
        builder.CreateRetVoid();
    }
}

void IREmitter::visitClass(ClassNode& node)
{
    // generate constructor
    if (node.hasCtor())
    {
        node.getCtor().accept(*this);
    }
    // generate methods
    for (MethodNode* method : node.getMethods())
    {
        method->accept(*this);
    }
    // generate destructor
    generateDtor(node);
}

void IREmitter::visitMethod(MethodNode& node)
{
    ClassNode& parent = node.getParent();
    // setup the body
    Value funcVal = global.getMethod(parent.getType(), node.getName()).first;
    if (!funcVal)
    {
        // something bad happened
        return;
    }
    setupFuncBody(node, funcVal);
    func.setReturnType(node.getReturnType());
    // add the self parameter
    self = Value::getExpr(parent.getType(), funcVal.getLLVMFunc()->arg_begin());
    // generate everything
    genFuncBody(node);
    cleanupFuncBody(node);
    self = Value::getNull();
}

void IREmitter::visitCtor(CtorNode& node)
{
    ClassNode& parent = node.getParent();
    // get a reference to the function declaration
    Value funcVal = global.getCtor(parent.getType()).first;
    assert(funcVal && "FuncResolver didn't run or didn't register ctor");
    // setup function
    setupFuncBody(node, funcVal);
    // constructors don't return values internally, which are instead created by
    //  the caller
    func.setReturnType(vslCtx.getVoidType());
    // add the self parameter
    self = Value::getExpr(parent.getType(), funcVal.getLLVMFunc()->arg_begin());
    // generate everything
    genFuncBody(node);
    cleanupFuncBody(node);
    self = Value::getNull();
}

void IREmitter::visitBlock(BlockNode& node)
{
    // tells us whether we need to destroy vars in our new scope
    bool returned = false;
    func.enter();
    for (Node* statement : node.getStatements())
    {
        // visit each statement
        if (returned)
        {
            // we've already hit a return statement, so all code after this is
            //  unreachable and shouldn't be bothered
            diag.print<Diag::UNREACHABLE>(*statement);
            break;
        }
        statement->accept(*this);
        // check if this block is returning
        if (statement->is(Node::RETURN))
        {
            returned = true;
        }
        // expression statements are discarded should one come up
        destroyValue(result);
    }
    // if there's no return, destroy vars in the current scope
    if (!returned)
    {
        destroyVars();
    }
    func.exit();
    result = Value::getNull();
}

void IREmitter::visitEmpty(EmptyNode& node)
{
    result = Value::getNull();
}

void IREmitter::visitIf(IfNode& node)
{
    // make sure that it's in a function
    if (func.empty())
    {
        diag.print<Diag::TOPLEVEL_CTRL_FLOW>(node.getLoc());
    }
    // setup the condition
    func.enter();
    node.getCondition().accept(*this);
    if (!result)
    {
        return;
    }
    Value condition;
    // make sure it's a bool
    if (result.getVSLType() == vslCtx.getBoolType())
    {
        condition = result;
    }
    else
    {
        // error, assume false
        diag.print<Diag::CANNOT_CONVERT>(node.getCondition(),
            *result.getVSLType(), *vslCtx.getBoolType());
        condition = Value::getExpr(vslCtx.getBoolType(), builder.getFalse());
    }
    // create the necessary basic blocks
    llvm::Function* currentFunc = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(llvmCtx, "if.then");
    auto* endBlock = llvm::BasicBlock::Create(llvmCtx, "if.end");
    llvm::BasicBlock* elseBlock;
    // if the condition is false...
    if (node.hasElse())
    {
        // ...branch to the else block
        elseBlock = llvm::BasicBlock::Create(llvmCtx, "if.else");
    }
    else
    {
        // ...branch to the cont block
        elseBlock = endBlock;
    }
    // create the branch instruction
    builder.CreateCondBr(loadValue(condition).getLLVMValue(), thenBlock,
        elseBlock);
    // generate then
    func.enter();
    thenBlock->insertInto(currentFunc);
    builder.SetInsertPoint(thenBlock);
    node.getThen().accept(*this);
    // cleanup
    destroyValue(result);
    if (node.getThen().isNot(Node::RETURN))
    {
        destroyVars();
    }
    branchTo(endBlock);
    func.exit();
    // generate else block if it exists
    if (node.hasElse())
    {
        // very similar to how the then case is generated
        func.enter();
        elseBlock->insertInto(currentFunc);
        builder.SetInsertPoint(elseBlock);
        node.getElse().accept(*this);
        // cleanup
        destroyValue(result);
        if (node.getElse().isNot(Node::RETURN))
        {
            destroyVars();
        }
        branchTo(endBlock);
        func.exit();
    }
    // setup end block for other code after the IfNode if needed
    // this handles when both then and else cases have a return statement
    if (!endBlock->use_empty())
    {
        // insert the end block
        endBlock->insertInto(currentFunc);
        builder.SetInsertPoint(endBlock);
        // cleanup
        destroyValue(condition);
        destroyVars();
    }
    else
    {
        // endBlock isn't being used so it should be destroyed
        // this should only happen if both then/else cases are either errored or
        //  have a return statement
        delete endBlock;
        // all code after this is unreachable
        builder.ClearInsertionPoint();
    }
    func.exit();
    result = Value::getNull();
}

void IREmitter::visitReturn(ReturnNode& node)
{
    // special case: return without a value
    if (!node.hasValue())
    {
        destroyAllVars();
        builder.CreateRetVoid();
        return;
    }
    // validate the return value
    node.getValue().accept(*this);
    Value value = copyValue(result);
    result = Value::getNull();
    // cleanup
    destroyAllVars();
    // type checking
    if (value)
    {
        if (value.getVSLType() != func.getReturnType())
        {
            diag.print<Diag::RETVAL_MISMATCHES_RETTYPE>(node.getValue(),
                *value.getVSLType(), *func.getReturnType());
        }
        else if (value.getVSLType() == vslCtx.getVoidType())
        {
            diag.print<Diag::CANT_RETURN_VOID_VALUE>(node);
        }
        else
        {
            // nothing bad happened yay
            builder.CreateRet(value.getLLVMValue());
            return;
        }
    }
    // errors in the return value expression create an unreachable instruction
    //  instead of the usual ret
    builder.CreateUnreachable();
}

void IREmitter::visitIdent(IdentNode& node)
{
    // just lookup the identifier
    result = lookupIdent(node);
}

void IREmitter::visitLiteral(LiteralNode& node)
{
    // create an LLVM integer
    unsigned width = node.getValue().getBitWidth();
    const Type* type;
    switch (width)
    {
    case 1:
        type = vslCtx.getBoolType();
        break;
    case 32:
        type = vslCtx.getIntType();
        break;
    default:
        // should never happen
        diag.print<Diag::INVALID_INT_WIDTH>(node);
        result = Value::getNull();
        return;
    }
    result = Value::getExpr(type,
        llvm::ConstantInt::get(llvmCtx, node.getValue()));
}

void IREmitter::visitUnary(UnaryNode& node)
{
    // verify the contained expression
    node.getExpr().accept(*this);
    if (!result)
    {
        return;
    }
    Value value = result;
    Value loaded = loadValue(value);
    // choose the appropriate operator to generate code for
    switch (node.getOp())
    {
    case UnaryKind::NOT:
        // should only be valid on booleans
        if (value.getVSLType() != vslCtx.getBoolType())
        {
            result = Value::getNull();
            break;
        }
        // fallthrough
    case UnaryKind::MINUS:
        genNeg(loaded);
        break;
    default:
        // should never happen
        result = Value::getNull();
    }
    if (!result)
    {
        diag.print<Diag::INVALID_UNARY>(node, *value.getVSLType());
    }
    // teardown
    destroyValue(value);
}

void IREmitter::visitBinary(BinaryNode& node)
{
    // special case: variable assignment
    if (node.getOp() == BinaryKind::ASSIGN)
    {
        genAssign(node);
        return;
    }
    // special case: short-circuiting boolean operations
    if (node.getOp() == BinaryKind::AND || node.getOp() == BinaryKind::OR)
    {
        genShortCircuit(node);
        return;
    }
    // verify the left and right expressions
    node.getLhs().accept(*this);
    if (!result)
    {
        return;
    }
    Value lhs = result;
    Value loadedLhs = loadValue(lhs);
    node.getRhs().accept(*this);
    if (!result)
    {
        destroyValue(lhs);
        return;
    }
    Value rhs = result;
    Value loadedRhs = loadValue(rhs);
    result = Value::getNull();
    // make sure the types match
    if (lhs.getVSLType() == rhs.getVSLType())
    {
        // choose the appropriate operator to generate code for
        const Type* type = lhs.getVSLType();
        llvm::Value* lhsVal = loadedLhs.getLLVMValue();
        llvm::Value* rhsVal = loadedRhs.getLLVMValue();
        switch (node.getOp())
        {
        case BinaryKind::PLUS:
            genAdd(type, lhsVal, rhsVal);
            break;
        case BinaryKind::MINUS:
            genSub(type, lhsVal, rhsVal);
            break;
        case BinaryKind::STAR:
            genMul(type, lhsVal, rhsVal);
            break;
        case BinaryKind::SLASH:
            genDiv(type, lhsVal, rhsVal);
            break;
        case BinaryKind::PERCENT:
            genMod(type, lhsVal, rhsVal);
            break;
        case BinaryKind::EQUAL:
            genEQ(type, lhsVal, rhsVal);
            break;
        case BinaryKind::NOT_EQUAL:
            genNE(type, lhsVal, rhsVal);
            break;
        case BinaryKind::GREATER:
            genGT(type, lhsVal, rhsVal);
            break;
        case BinaryKind::GREATER_EQUAL:
            genGE(type, lhsVal, rhsVal);
            break;
        case BinaryKind::LESS:
            genLT(type, lhsVal, rhsVal);
            break;
        case BinaryKind::LESS_EQUAL:
            genLE(type, lhsVal, rhsVal);
            break;
        default:
            ; // should never happen
        }
    }
    if (!result)
    {
        diag.print<Diag::INVALID_BINARY>(node, *lhs.getVSLType(),
            *rhs.getVSLType());
    }
    // teardown
    destroyValue(lhs);
    destroyValue(rhs);
}

void IREmitter::visitTernary(TernaryNode& node)
{
    // generate condition and make sure its a bool
    node.getCondition().accept(*this);
    if (!result)
    {
        return;
    }
    if (result.getVSLType() != vslCtx.getBoolType())
    {
        diag.print<Diag::CANNOT_CONVERT>(node.getCondition(),
            *result.getVSLType(), *vslCtx.getBoolType());
        return;
    }
    Value condition = result;
    // setup blocks
    llvm::BasicBlock* currBlock = builder.GetInsertBlock();
    llvm::Function* currFunc = currBlock->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(llvmCtx, "ternary.then");
    auto* elseBlock = llvm::BasicBlock::Create(llvmCtx, "ternary.else");
    auto* contBlock = llvm::BasicBlock::Create(llvmCtx, "ternary.cont");
    // branch based on the condition
    builder.CreateCondBr(loadValue(condition).getLLVMValue(), thenBlock,
        elseBlock);
    // generate then
    thenBlock->insertInto(currFunc);
    builder.SetInsertPoint(thenBlock);
    node.getThen().accept(*this);
    Value thenCase = copyValue(result);
    // make sure thenCase is valid before continuing
    if (!thenCase)
    {
        // something bad happened
        delete contBlock;
        return;
    }
    // we get a reference to the current block after generating code because an
    //  expression can span multiple basic blocks, e.g. a ternary such as this
    llvm::BasicBlock* thenEnd = builder.GetInsertBlock();
    branchTo(contBlock);
    // generate else
    elseBlock->insertInto(currFunc);
    builder.SetInsertPoint(elseBlock);
    node.getElse().accept(*this);
    Value elseCase = copyValue(result);
    // make sure elseCase is valid before continuing
    if (!elseCase)
    {
        return;
    }
    llvm::BasicBlock* elseEnd = builder.GetInsertBlock();
    branchTo(contBlock);
    // setup cont block for code that comes after
    contBlock->insertInto(currFunc);
    builder.SetInsertPoint(contBlock);
    // do type checking to make sure everything's fine
    if (thenCase.getVSLType() != elseCase.getVSLType())
    {
        diag.print<Diag::TERNARY_TYPE_MISMATCH>(node, *thenCase.getVSLType(),
            *elseCase.getVSLType());
        result = Value::getNull();
        return;
    }
    // bring it all together with a phi node
    auto* phi = builder.CreatePHI(thenCase.getLLVMValue()->getType(), 2,
        "ternary.phi");
    phi->addIncoming(thenCase.getLLVMValue(), thenEnd);
    phi->addIncoming(elseCase.getLLVMValue(), elseEnd);
    result = Value::getExpr(thenCase.getVSLType(), phi);
    // teardown
    destroyValue(condition);
}

void IREmitter::visitCall(CallNode& node)
{
    // make sure the callee is an actual function
    node.getCallee().accept(*this);
    if (!result)
    {
        return;
    }
    Value callee = result;
    if (!callee.isFunc())
    {
        diag.print<Diag::NOT_A_FUNCTION>(node.getCallee(),
            *callee.getVSLType());
        result = Value::getNull();
        return;
    }
    // call the function
    createCall(node, loadValue(callee));
    // teardown
    destroyValue(callee);
}

void IREmitter::visitArg(ArgNode& node)
{
    node.getValue().accept(*this);
}

void IREmitter::visitFieldAccess(FieldAccessNode& node)
{
    // evaluate the object
    node.getObject().accept(*this);
    if (!result)
    {
        // something bad happened while trying to resolve the object
        return;
    }
    Value base = result;
    // resolve the type
    const Type* type = base.getVSLType();
    const ClassType* classType = toClassType(type);
    // field access applies only to objects of class type
    if (!classType)
    {
        diag.print<Diag::NOT_AN_OBJECT>(node.getObject(), *type);
        result = Value::getNull();
        return;
    }
    // get the field type+offset
    ClassType::Field field = classType->getField(node.getField());
    // verify that the field exists and can be accessed
    if (!field)
    {
        diag.print<Diag::UNKNOWN_FIELD>(node, *type);
        result = Value::getNull();
        return;
    }
    if (!canAccessMember(type, field.access))
    {
        // can't access private fields outside of methods/ctors
        diag.print<Diag::PRIVATE_FIELD>(node, *type);
        result = Value::getNull();
        return;
    }
    // create a gep instruction to access the field
    // class types always start with a ptr to the refcounted struct
    Value baseLoaded = loadValue(base);
    llvm::Value* objPtr = baseLoaded.getLLVMValue();
    std::initializer_list<llvm::Value*> indexes
    {
        // array index: %A* -> %A*
        createGEPIndex(objPtr->getType(), 0),
        // struct index: %A* -> %struct.A*
        builder.getInt32(1),
        // field index: %struct.A* -> %<field type>
        builder.getInt32(field.index)
    };
    llvm::Value* gep = builder.CreateGEP(objPtr, indexes);
    // create the field Value
    bool destroyBase;
    if (base.isField())
    {
        // field of a field Value should use the base of the base
        destroyBase = base.shouldDestroyBase();
        base = base.getBase();
    }
    else
    {
        // can destroy base if it's an expr that isn't the self param
        destroyBase = base.isExpr() && base != self;
        base = baseLoaded;
    }
    // at this point, base should be an expr Value
    result = Value::getField(base, field.type, gep, destroyBase);
}

void IREmitter::visitMethodCall(MethodCallNode& node)
{
    // get the object to use as the self parameter
    node.getCallee().accept(*this);
    if (!result)
    {
        return;
    }
    Value selfArg = result;
    // lookup the method
    Value methodFunc;
    Access access;
    std::tie(methodFunc, access) =
        global.getMethod(selfArg.getVSLType(), node.getMethod());
    if (!methodFunc)
    {
        // method can't be found
        diag.print<Diag::UNKNOWN_METHOD>(node, *selfArg.getVSLType());
        result = Value::getNull();
        return;
    }
    if (!canAccessMember(selfArg.getVSLType(), access))
    {
        // method can't be accessed
        diag.print<Diag::PRIVATE_METHOD>(node, *selfArg.getVSLType());
        result = Value::getNull();
        return;
    }
    // call the method in a similar manner to CallNode except with the self arg
    createCall(node, methodFunc, loadValue(selfArg));
    // teardown
    destroyValue(selfArg);
}

void IREmitter::visitSelf(SelfNode& node)
{
    result = self;
    if (!result)
    {
        diag.print<Diag::SELF_NOT_DEFINED>(node);
    }
}

void IREmitter::genNeg(Value value)
{
    const Type* type = value.getVSLType();
    result = (type == vslCtx.getIntType() || type == vslCtx.getBoolType()) ?
        Value::getExpr(type, builder.CreateNeg(value.getLLVMValue(), "neg")) :
        Value::getNull();
}

void IREmitter::genAssign(BinaryNode& node)
{
    ExprNode& lhs = node.getLhs();
    ExprNode& rhs = node.getRhs();
    // evaluate rhs first
    rhs.accept(*this);
    Value rhsVal = result;
    // copy first to separate lhs and rhs code
    Value rhsCopy = copyValue(rhsVal);
    result = Value::getNull();
    // then try to evaluate lhs
    lhs.accept(*this);
    Value lhsVal = result;
    result = Value::getNull();
    // verify the lhs
    if (!lhsVal)
    {
        return;
    }
    // make sure the lhs is assignable
    if (!lhsVal.isAssignable())
    {
        diag.print<Diag::LHS_NOT_ASSIGNABLE>(lhs);
        return;
    }
    // verify the rhs
    if (!rhsVal)
    {
        return;
    }
    // make sure the types match up before doing the assignment
    if (lhsVal.getVSLType() != rhsVal.getVSLType())
    {
        diag.print<Diag::CANNOT_CONVERT>(rhs, *rhsVal.getVSLType(),
            *lhsVal.getVSLType());
        return;
    }
    // finally, create the store instruction
    storeValue(rhsCopy, lhsVal);
}

void IREmitter::genShortCircuit(BinaryNode& node)
{
    /*
     * End result should be something like this:
     *
     * currBlock:
     * calc cond1
     * br cond1, longCheck, cont // if and
     * br cond1, cont, longCheck // if or
     *
     * longCheck:
     * calc cond2
     * br cont
     *
     * cont:
     * phi [false, currBlock], [cond2, longCheck] // if and
     * phi [true, currBlock], [cond2, longCheck] // if or
     */
    // generate code to calculate cond1 (lhs)
    ExprNode& lhs = node.getLhs();
    lhs.accept(*this);
    if (!result)
    {
        return;
    }
    Value cond1 = result;
    Value cond1Loaded = loadValue(cond1);
    // of course, lhs has to be a bool for this to work
    if (cond1.getVSLType() != vslCtx.getBoolType())
    {
        diag.print<Diag::CANNOT_CONVERT>(lhs, *cond1.getVSLType(),
            *vslCtx.getBoolType());
        result = Value::getNull();
        destroyValue(cond1);
        return;
    }
    // helper variables so i don't have to type as much
    auto* currBlock = builder.GetInsertBlock();
    llvm::Function* currFunc = currBlock->getParent();
    // setup blocks
    llvm::Twine name = (node.getOp() == BinaryKind::AND) ? "and" : "or";
    auto* longCheck = llvm::BasicBlock::Create(llvmCtx, name + ".long");
    auto* cont = llvm::BasicBlock::Create(llvmCtx, name + ".cont");
    // create the branch
    if (node.getOp() == BinaryKind::AND)
    {
        builder.CreateCondBr(cond1Loaded.getLLVMValue(), longCheck, cont);
    }
    else // or
    {
        builder.CreateCondBr(cond1Loaded.getLLVMValue(), cont, longCheck);
    }
    // the long check is when the operation did not short circuit, and depends
    //  on the value of rhs to fully determine the result
    longCheck->insertInto(currFunc);
    builder.SetInsertPoint(longCheck);
    // generate code to calculate cond2 (rhs)
    ExprNode& rhs = node.getRhs();
    rhs.accept(*this);
    Value cond2 = result;
    if (!cond2)
    {
        destroyValue(cond1);
        return;
    }
    Value cond2Loaded = loadValue(cond2);
    // setup the cont block
    branchTo(cont);
    cont->insertInto(currFunc);
    builder.SetInsertPoint(cont);
    // of course, rhs has to be a bool for this to work
    // the check happens later so that the cont block is neither a memory leak
    //  nor a dangling pointer, and we can safely insert other code afterwards
    if (result.getVSLType() != vslCtx.getBoolType())
    {
        diag.print<Diag::CANNOT_CONVERT>(rhs, *cond2.getVSLType(),
            *vslCtx.getBoolType());
        result = Value::getNull();
        destroyValue(cond1);
        destroyValue(cond2);
        return;
    }
    // create the phi instruction
    auto* phi = builder.CreatePHI(builder.getInt1Ty(), 2, name);
    // if the branch came from currBlock, then the operation short-circuited
    phi->addIncoming(builder.getInt1(node.getOp() == BinaryKind::OR),
        currBlock);
    // if it came from longCheck, then the result is determined by rhs
    phi->addIncoming(cond2Loaded.getLLVMValue(), longCheck);
    // teardown
    destroyValue(cond1);
    destroyValue(cond2);
    result = Value::getExpr(vslCtx.getBoolType(), phi);
}

void IREmitter::genAdd(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        Value::getExpr(type, builder.CreateAdd(lhs, rhs, "add")) :
        Value::getNull();
}

void IREmitter::genSub(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        Value::getExpr(type, builder.CreateSub(lhs, rhs, "sub")) :
        Value::getNull();
}

void IREmitter::genMul(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        Value::getExpr(type, builder.CreateMul(lhs, rhs, "mul")) :
        Value::getNull();
}

void IREmitter::genDiv(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        Value::getExpr(type, builder.CreateSDiv(lhs, rhs, "sdiv")) :
        Value::getNull();
}

void IREmitter::genMod(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        Value::getExpr(type, builder.CreateSRem(lhs, rhs, "srem")) :
        Value::getNull();
}

void IREmitter::genEQ(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType() || type == vslCtx.getBoolType()) ?
        Value::getExpr(vslCtx.getBoolType(),
            builder.CreateICmpEQ(lhs, rhs, "cmp")) :
        Value::getNull();
}

void IREmitter::genNE(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType() || type == vslCtx.getBoolType()) ?
        Value::getExpr(vslCtx.getBoolType(),
            builder.CreateICmpNE(lhs, rhs, "cmp")) :
        Value::getNull();
}

void IREmitter::genGT(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        Value::getExpr(vslCtx.getBoolType(),
            builder.CreateICmpSGT(lhs, rhs, "cmp")) :
        Value::getNull();
}

void IREmitter::genGE(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        Value::getExpr(vslCtx.getBoolType(),
            builder.CreateICmpSGE(lhs, rhs, "cmp")) :
        Value::getNull();
}

void IREmitter::genLT(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        Value::getExpr(vslCtx.getBoolType(),
            builder.CreateICmpSLT(lhs, rhs, "cmp")) :
        Value::getNull();
}

void IREmitter::genLE(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        Value::getExpr(vslCtx.getBoolType(),
            builder.CreateICmpSLE(lhs, rhs, "cmp")) :
        Value::getNull();
}

llvm::AllocaInst* IREmitter::createEntryAlloca(llvm::Type* type,
    const llvm::Twine& name)
{
    auto ip = builder.saveIP();
    if (!allocaInsertPoint)
    {
        // create a no-op at the beginning of the entry block as a reference for
        //  allocas to be inserted before so that they're in order
        llvm::Value* zero = builder.getFalse();
        allocaInsertPoint = llvm::BinaryOperator::Create(llvm::Instruction::Add,
            zero, zero, "allocapoint");
        llvm::BasicBlock* entry = &ip.getBlock()->getParent()->getEntryBlock();
        entry->getInstList().push_front(allocaInsertPoint);
    }
    builder.SetInsertPoint(allocaInsertPoint);
    llvm::AllocaInst* inst = builder.CreateAlloca(type, nullptr, name);
    builder.restoreIP(ip);
    return inst;
}

llvm::BranchInst* IREmitter::branchTo(llvm::BasicBlock* target)
{
    if (builder.GetInsertBlock() && !builder.GetInsertBlock()->getTerminator())
    {
        return builder.CreateBr(target);
    }
    return nullptr;
}

bool IREmitter::isGlobal() const
{
    return func.empty();
}

llvm::GlobalVariable* IREmitter::genGlobalVar(Access access,
    const Type* vslType, llvm::Type* llvmType, llvm::StringRef name)
{
    // determine the linkage type
    llvm::GlobalValue::LinkageTypes linkage = accessToLinkage(access);
    // create a placeholder initializer value
    llvm::Constant* initializer = llvm::Constant::getNullValue(llvmType);
    // create the variable
    auto* var = new llvm::GlobalVariable{ module, llvmType,
        /*isConstant=*/false, linkage, initializer, name };
    // add to global scope
    if (global.setVar(name, vslType, var))
    {
        // variable already exists
        var->eraseFromParent();
        return nullptr;
    }
    return var;
}

llvm::Function* IREmitter::genGlobalVarCtor(llvm::GlobalVariable* var)
{
    // create the constructor function
    auto* funcType = llvm::FunctionType::get(builder.getVoidTy(),
        /*isVarArg=*/false);
    auto* globalVarCtor = llvm::Function::Create(funcType,
        llvm::GlobalValue::InternalLinkage, var->getName() + ".ctor",
        &module);
    // call the ctor function at program start
    addGlobalCtor(globalVarCtor);
    // create the entry block
    llvm::BasicBlock::Create(llvmCtx, "entry", globalVarCtor);
    // user must fill in other code
    return globalVarCtor;
}

void IREmitter::genGlobalVarDtor(llvm::GlobalVariable* var, const Type* type)
{
    llvm::Function* dtorFunc = global.getDtor(type);
    if (!dtorFunc)
    {
        // destructor doesn't even exist!
        // generating a global var destructor function is unnecessary
        return;
    }
    // create the global var destructor function
    auto* funcType = llvm::FunctionType::get(builder.getVoidTy(),
        /*isVarArg=*/false);
    auto* globalVarDtor = llvm::Function::Create(funcType,
        llvm::GlobalValue::InternalLinkage, var->getName() + ".dtor",
        &module);
    // call that function at program start
    addGlobalDtor(globalVarDtor);
    // create the entry block
    auto* entry = llvm::BasicBlock::Create(llvmCtx, "entry", globalVarDtor);
    builder.SetInsertPoint(entry);
    // load the global variable
    llvm::Value* varValue = builder.CreateLoad(var);
    // call the type's destructor function
    builder.CreateCall(dtorFunc, { varValue });
    // terminate the destructor function
    builder.CreateRetVoid();
}

void IREmitter::addGlobalCtor(llvm::Function* f)
{
    addGlobalCall(f, /*startOrEnd=*/true);
}

void IREmitter::addGlobalDtor(llvm::Function* f)
{
    addGlobalCall(f, /*startOrEnd=*/false);
}

void IREmitter::addGlobalCall(llvm::Function* f, bool startOrEnd)
{
    // get/create the global vsl initialization function
    auto* funcType = llvm::FunctionType::get(builder.getVoidTy(),
        /*isVarArg=*/false);
    llvm::BasicBlock* insertBlock;
    // select the function to create
    llvm::Function* globalFunc;
    llvm::StringRef name;
    std::tie(globalFunc, name) = startOrEnd ?
        std::forward_as_tuple(vslCtor, "ctors") :
        std::forward_as_tuple(vslDtor, "dtors");
    if (globalFunc)
    {
        // global ctor/dtor func already exists
        insertBlock = &globalFunc->back();
    }
    else
    {
        // global ctor/dtor function doesn't already exist so create it now
        globalFunc = llvm::Function::Create(funcType,
            llvm::GlobalValue::InternalLinkage, "vsl." + name, &module);
        // terminate the function with a void return
        insertBlock = llvm::BasicBlock::Create(llvmCtx, "entry", globalFunc);
        builder.SetInsertPoint(insertBlock);
        builder.CreateRetVoid();
        // create the @llvm.global_<name> intrinsic variable so that the global
        //  ctor/dtor func gets called at runtime
        /*
         * End result:
         * %0 = type { i32, void ()*, i8* }
         * @llvm.global_<name> = appending global [1 x %0]
         *     [%0 { i32 65535, void ()* @vsl.<name>, i8* null }]
         */
        // create the required llvm::Type objects
        auto* priorityType = builder.getInt32Ty();
        auto* dataType = builder.getInt8PtrTy();
        auto* ctorType = llvm::StructType::create("", priorityType,
            funcType->getPointerTo(), dataType);
        auto* ctorArrayType = llvm::ArrayType::get(ctorType, 1);
        // create the initializer object
        auto* ctor = llvm::ConstantStruct::get(ctorType,
            builder.getInt32(65535), globalFunc,
            llvm::ConstantPointerNull::get(dataType));
        auto* ctorArray = llvm::ConstantArray::get(ctorArrayType, ctor);
        // create the variable, which is automatically added to the module
        new llvm::GlobalVariable{ module, ctorArrayType, /*isConstant=*/false,
            llvm::GlobalValue::AppendingLinkage, ctorArray,
            "llvm.global_" + name };
    }
    // insert all function calls before the return instruction
    builder.SetInsertPoint(insertBlock->getTerminator());
    // call the given global ctor function
    assert(f->getFunctionType() == funcType &&
        "Invalid global ctor type! Should be void ().");
    builder.CreateCall(f);
}

llvm::Value* IREmitter::createMalloc(const Type* type)
{
    const Type* newType = type;
    llvm::StringRef name;
    // resolve the type
    if (newType->is(Type::NAMED))
    {
        // high-level name of the type is known, but not the underlying type
        name = static_cast<const NamedType*>(newType)->getName();
        // iteratively get the most underlying type
        do
        {
            newType =
                static_cast<const NamedType*>(newType)->getUnderlyingType();
        }
        while (newType->is(Type::NAMED));
    }
    // do something special for mallocing a class
    if (newType->is(Type::CLASS))
    {
        return createMalloc(static_cast<const ClassType*>(newType), name);
    }
    // or not if it isn't a class
    return createMalloc(converter.convert(newType), name);
}

llvm::Value* IREmitter::createMalloc(const ClassType* type,
    llvm::StringRef name)
{
    // get the pointer type of the object
    llvm::PointerType* objPtrType = converter.convert(type);
    // object type to allocate
    auto* objType = llvm::cast<llvm::StructType>(objPtrType->getElementType());
    // create the malloc call
    llvm::Value* obj = createMalloc(objType, llvm::Twine{ "obj." } + name);
    if (!obj)
    {
        return nullptr;
    }
    // get the type of a getelementptr index
    llvm::IntegerType* indexType =
        module.getDataLayout().getIntPtrType(llvmCtx);
    // create list of getelementptr indexes
    std::initializer_list<llvm::Value*> indexes
    {
        // index as array
        llvm::ConstantInt::get(indexType, 0),
        // index as struct (for the refcount): %A* -> i32*
        // after the index, only i32 constants are allowed
        builder.getInt32(0)
    };
    // get a pointer to the object's reference count
    llvm::Value* refcount = builder.CreateGEP(objType, obj, indexes,
        llvm::Twine{ "obj." } + name + ".refcount");
    // initialize the refcount with a 1, so the object is now live and ready to
    //  be initialized
    builder.CreateStore(builder.getInt32(1), refcount);
    return obj;
}

llvm::Value* IREmitter::createMalloc(llvm::Type* type, const llvm::Twine& name)
{
    if (!builder.GetInsertBlock())
    {
        // builder not able to insert any instructions!
        return nullptr;
    }
    // int type large enough to hold a pointer
    llvm::IntegerType* intPtrType = builder.getIntPtrTy(module.getDataLayout());
    // compute the size of the struct type
    auto* allocSize = llvm::ConstantInt::get(intPtrType,
        module.getDataLayout().getTypeAllocSize(type));
    // call malloc/bitcast
    if (builder.GetInsertPoint() == builder.GetInsertBlock()->end())
    {
        // builder is currently inserting at the end of a block
        return builder.Insert(llvm::CallInst::CreateMalloc(
                builder.GetInsertBlock(), intPtrType, type, allocSize,
                /*ArraySize=*/nullptr, /*MallocF=*/nullptr), name);
    }
    // builder is currently inserting inside of a block
    return builder.Insert(llvm::CallInst::CreateMalloc(
            &*builder.GetInsertPoint(), intPtrType, type, allocSize,
            /*ArraySize=*/nullptr, /*MallocF=*/nullptr), name);
}

Value IREmitter::lookupIdent(IdentNode& node)
{
    // try to get it from the function scope first
    Value value = func.get(node.getName());
    if (!value)
    {
        // maybe global scope?
        value = global.get(node.getName());
        if (!value)
        {
            // maybe constructor?
            const NamedType* selfType = vslCtx.getNamedType(node.getName());
            Access access;
            std::tie(value, access) = global.getCtor(selfType);
            if (!value)
            {
                // doesn't exist!
                diag.print<Diag::UNKNOWN_IDENT>(node);
            }
            else if (!canAccessMember(selfType, access))
            {
                // constructor can't be accessed!
                diag.print<Diag::PRIVATE_CTOR>(node);
                value = Value::getNull();
            }
        }
    }
    return value;
}

llvm::Constant* IREmitter::createGEPIndex(llvm::Type* ptrType,
    uint64_t i) const
{
    // in llvm 7, use getIndexType
    return llvm::ConstantInt::get(module.getDataLayout().getIntPtrType(ptrType),
        i);
}

void IREmitter::setupFuncBody(FunctionNode& node, Value funcVal)
{
    if (node.isAlreadyDefined())
    {
        // probably flagged by FuncResolver
        return;
    }
    assert(funcVal.isFunc() &&
        "FuncResolver didn't run or didn't register function");
    // setup function scope
    assert(func.empty() && "parser didn't reject func within func");
    func.enter();
    // create the entry block
    const FunctionType* funcType = funcVal.getVSLFunc();
    llvm::Function* llvmFunc = funcVal.getLLVMFunc();
    auto* entry = llvm::BasicBlock::Create(llvmCtx, "entry", llvmFunc);
    builder.SetInsertPoint(entry);
    // setup parameter list
    auto argIt = llvmFunc->arg_begin();
    if (funcType->hasSelfType() || funcType->isCtor())
    {
        // methods/constructors take an implicit self parameter at the beginning
        //  so skip that
        ++argIt;
    }
    for (size_t i = 0; i < node.getNumParams(); ++i, ++argIt)
    {
        // get the vsl and llvm parameter representation
        const ParamNode& param = node.getParam(i);
        llvm::Argument* llvmParam = &*argIt;
        // load the parameter into a runtime variable
        llvm::Value* alloca = createEntryAlloca(llvmParam->getType(),
            param.getName());
        builder.CreateStore(llvmParam, alloca);
        // add that variable to function scope
        func.set(param.getName(), Value::getVar(param.getType(), alloca));
    }
}

void IREmitter::genFuncBody(FunctionNode& node)
{
    // generate the body
    node.getBody().accept(*this);
}

void IREmitter::cleanupFuncBody(FunctionNode& node)
{
    // make sure the last block is terminated and not waiting to insert anymore
    //  instructions afterward
    auto* bb = builder.GetInsertBlock();
    if (bb && !bb->getTerminator())
    {
        destroyAllVars();
        if (func.getReturnType() == vslCtx.getVoidType())
        {
            builder.CreateRetVoid();
        }
        else
        {
            diag.print<Diag::MISSING_RETURN>(node);
            builder.CreateUnreachable();
        }
    }
    // prevent additional instructions from being inserted
    builder.ClearInsertionPoint();
    // exit the current scope
    func.exit();
    // erase the alloca point because nobody needs to see it
    if (allocaInsertPoint)
    {
        allocaInsertPoint->eraseFromParent();
        allocaInsertPoint = nullptr;
    }
    result = Value::getNull();
}

void IREmitter::createCall(CallNode& node, Value funcVal, Value selfArg)
{
    const FunctionType* calleeType = funcVal.getVSLFunc();
    // make sure the right amount of arguments is used
    if (calleeType->getNumParams() != node.getNumArgs())
    {
        diag.print<Diag::MISMATCHING_ARG_COUNT>(node.getLoc(),
            node.getNumArgs(), calleeType->getNumParams());
        result = Value::getNull();
        return;
    }
    const Type* retType = calleeType->getReturnType();
    llvm::Function* func = funcVal.getLLVMFunc();
    // setup vsl arguments list
    std::vector<Value> vslArgs;
    vslArgs.reserve(calleeType->getNumParams());
    // setup llvm arguments list
    std::vector<llvm::Value*> llvmArgs;
    llvm::Value* llvmSelfArg = nullptr;
    if (calleeType->hasSelfType())
    {
        // add the implicit self parameter
        if (calleeType->isCtor())
        {
            // ctors require the caller to malloc the object first
            llvmSelfArg = createMalloc(calleeType->getSelfType());
        }
        else if (calleeType->isMethod())
        {
            // methods just require the self argument
            assert(selfArg.getVSLType() == calleeType->getSelfType() &&
                "invalid self param!");
            llvmSelfArg = selfArg.getLLVMValue();
        }
        // reserve a space for the self parameter
        llvmArgs.reserve(calleeType->getNumParams() + 1);
        llvmArgs.push_back(llvmSelfArg);
    }
    else
    {
        // reserve the normal amount of memory
        llvmArgs.reserve(calleeType->getNumParams());
    }
    // verify each argument
    bool valid = true;
    for (size_t i = 0; i < calleeType->getNumParams(); ++i)
    {
        const Type* paramType = calleeType->getParamType(i);
        ArgNode& arg = node.getArg(i);
        arg.accept(*this);
        // save the argument for destruction later
        vslArgs.push_back(result);
        // check that the types match
        if (result.getVSLType() == paramType)
        {
            llvmArgs.push_back(copyValue(result).getLLVMValue());
        }
        else
        {
            // print error diagnostics as long as the argument itself isn't the
            //  source of error
            if (result)
            {
                diag.print<Diag::CANNOT_CONVERT>(arg.getValue(),
                    *result.getVSLType(), *paramType);
            }
            valid = false;
        }
    }
    // create the call instruction if everything's valid
    if (valid)
    {
        llvm::Value* llvmVal = builder.CreateCall(func, llvmArgs);
        if (calleeType->isCtor())
        {
            if (!llvmSelfArg)
            {
                // something bad happened
                result = Value::getNull();
                return;
            }
            // self object was malloc'd previously since this is a constructor
            // use this as the actual result, not the call instruction
            llvmVal = llvmSelfArg;
        }
        result = Value::getExpr(retType, llvmVal);
    }
    else
    {
        result = Value::getNull();
    }
    // destroy each argument now that they've been used already
    for (Value argValue : vslArgs)
    {
        destroyValue(argValue);
    }
}

bool IREmitter::canAccessMember(const Type* objType, Access access) const
{
    return access != Access::PRIVATE || objType == self.getVSLType();
}

void IREmitter::generateDtor(const ClassNode& node)
{
    // get the function
    llvm::Function* llvmFunc = global.getDtor(node.getType());
    assert(llvmFunc && "FuncResolver didn't run or didn't register dtor");
    // setup the function
    auto* entry = llvm::BasicBlock::Create(llvmCtx, "entry", llvmFunc);
    builder.SetInsertPoint(entry);
    llvm::Value* objPtr = &*llvmFunc->arg_begin(); // first arg is always `self`
    // get the refcount
    std::initializer_list<llvm::Value*> indexes
    {
        // array index: %A* -> %A*
        createGEPIndex(objPtr->getType(), 0),
        // refcount index: %A* -> i32*
        builder.getInt32(0)
    };
    llvm::Value* rcPtr = builder.CreateGEP(objPtr, indexes,
        node.getName() + ".refcount");
    llvm::Value* rc = builder.CreateLoad(rcPtr);
    // subtract 1 from the refcount
    rc = builder.CreateSub(rc, builder.getInt32(1));
    // branch if the refcount is zero
    auto* dead = llvm::BasicBlock::Create(llvmCtx, "dead", llvmFunc);
    auto* alive = llvm::BasicBlock::Create(llvmCtx, "alive", llvmFunc);
    llvm::Value* isDead = builder.CreateICmpEQ(rc, builder.getInt32(0),
        "is_dead");
    builder.CreateCondBr(isDead, dead, alive);
    // if so, call the destructor of every field
    builder.SetInsertPoint(dead);
    for (size_t i = 0; i < node.getNumFields(); ++i)
    {
        // lookup the field's destructor
        const FieldNode& field = node.getField(i);
        llvm::Function* dtorFunc = global.getDtor(field.getType());
        if (!dtorFunc)
        {
            // destructor doesn't exist so this can be skipped
            continue;
        }
        // load the field
        std::initializer_list<llvm::Value*> fieldIndexes
        {
            // array index: %A* -> %A*
            createGEPIndex(objPtr->getType(), 0),
            // struct index: %A* -> %struct.A*
            builder.getInt32(1),
            // field index: %struct.A* -> <field type>*
            builder.getInt32(i)
        };
        llvm::Value* fieldPtr = builder.CreateGEP(objPtr, fieldIndexes,
            node.getName() + llvm::Twine{ '.' } + field.getName());
        llvm::Value* fieldValue = builder.CreateLoad(fieldPtr);
        // call the destructor
        builder.CreateCall(dtorFunc, { fieldValue });
    }
    // then, after destroying every field, free the allocated memory and return
    builder.Insert(llvm::CallInst::CreateFree(objPtr,
            builder.GetInsertBlock()));
    builder.CreateRetVoid();
    // if the refcount isn't zero, then update the refcount field and return
    builder.SetInsertPoint(alive);
    builder.CreateStore(rc, rcPtr);
    builder.CreateRetVoid();
}

const ClassType* IREmitter::toClassType(const Type* type)
{
    // infinitely get the underlying type until it's no longer a NamedType
    // TODO: protect against infinite loops
    // or just redo the type system
    while (type->is(Type::NAMED) &&
        static_cast<const NamedType*>(type)->hasUnderlyingType())
    {
        type = static_cast<const NamedType*>(type)->getUnderlyingType();
    }
    // make sure this is a ClassType before casting
    if (!type->is(Type::CLASS))
    {
        return nullptr;
    }
    return static_cast<const ClassType*>(type);
}

Value IREmitter::copyValue(Value value)
{
    // only assignable values (lvalues) can be copied
    // exprs (rvalues) are temporaries and can just be moved without doing
    //  anything else
    if (!value || !value.isAssignable())
    {
        return value;
    }
    // note that since this is assignable, we have a pointer to the object
    //  reference, therefore we need to load it first
    Value loaded = loadValue(value);
    // see if we have an object, since objects need to increment their refcount
    if (toClassType(value.getVSLType()))
    {
        llvm::Value* objPtr = loaded.getLLVMValue();
        // get the refcount
        std::initializer_list<llvm::Value*> rcIndexes
        {
            // array index: %A* -> %A*
            createGEPIndex(objPtr->getType(), 0),
            // refcount index: %A* -> i32*
            builder.getInt32(0)
        };
        llvm::Value* rcPtr = builder.CreateGEP(objPtr, rcIndexes, "refcount");
        // increment it
        llvm::Value* rc = builder.CreateLoad(rcPtr);
        rc = builder.CreateAdd(rc, builder.getInt32(1));
        // store the result
        builder.CreateStore(rc, rcPtr);
    }
    // do any cleanup if needed
    if (value.isField() && value.shouldDestroyBase())
    {
        // while the base object is destroyed, the field stays alive because it
        //  was already loaded and had its refcount updated (if needed)
        destroyValue(value.getBase());
    }
    return loaded;
}

void IREmitter::destroyValue(Value value)
{
    // destruction applies only to exprs/fields
    // if you need to destroy a variable value, call loadValue first
    if (!value || value.isVar())
    {
        return;
    }
    if (value.isField() && value.shouldDestroyBase())
    {
        // if we just destroy the field, the base object could potentially be a
        //  memory leak
        // since the base object's destructor should destroy all of its fields
        //  in addition to itself, we should only call the destructor.
        destroyValue(value.getBase());
        return;
    }
    // see if we actually have a destructor to call
    llvm::Function* llvmFunc = global.getDtor(value.getVSLType());
    if (!llvmFunc)
    {
        return;
    }
    // call that destructor
    builder.CreateCall(llvmFunc, { value.getLLVMValue() });
}

Value IREmitter::loadValue(Value value)
{
    // only assignable Values need to be loaded since they are pointers
    // regular expr Values can just be passed normally
    if (!value.isAssignable())
    {
        return value;
    }
    // load the value
    llvm::Value* load = builder.CreateLoad(value.getLLVMValue());
    return Value::getExpr(value.getVSLType(), load);
}

void IREmitter::storeValue(Value from, Value to)
{
    assert(from.isExpr() && "Not an expr!");
    assert(to.isAssignable() && "Not assignable!");
    // create the store instruction
    builder.CreateStore(from.getLLVMValue(), to.getLLVMValue());
    // do any cleanup if needed
    if (to.isField() && to.shouldDestroyBase())
    {
        destroyValue(to.getBase());
    }
}

void IREmitter::destroyVars()
{
    // go through the current scope to destroy vars
    llvm::ArrayRef<FuncScope::VarItem> scope = func.getVars();
    // start from the newest var
    for (auto varIt = scope.rbegin(); varIt != scope.rend(); ++varIt)
    {
        // run destructor
        // guaranteed to be a variable Value so need to load first
        destroyValue(loadValue(varIt->second));
    }
}

void IREmitter::destroyAllVars()
{
    std::vector<llvm::ArrayRef<FuncScope::VarItem>> allVars =
        func.getAllVars();
    // start from the newest scope...
    for (auto scopeIt = allVars.rbegin(); scopeIt != allVars.rend(); ++scopeIt)
    {
        llvm::ArrayRef<FuncScope::VarItem> scope = *scopeIt;
        // ...and newest var
        for (auto varIt = scope.rbegin(); varIt != scope.rend(); ++varIt)
        {
            destroyValue(loadValue(varIt->second));
        }
    }
}
