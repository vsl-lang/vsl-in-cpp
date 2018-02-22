#include "ast/opKind.hpp"
#include "irgen/passes/irEmitter/irEmitter.hpp"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <iterator>
#include <limits>

IREmitter::IREmitter(VSLContext& vslCtx, Diag& diag, FuncScope& func,
    GlobalScope& global, llvm::Module& module)
    : vslCtx{ vslCtx }, diag{ diag }, func{ func }, global{ global },
    module{ module }, llvmCtx{ module.getContext() }, builder{ llvmCtx },
    allocaInsertPoint{ nullptr }, vslInit{ nullptr }, result{ Value::getNull() }
{
}

void IREmitter::visitFunction(FunctionNode& node)
{
    if (node.isAlreadyDefined())
    {
        // probably flagged by FuncResolver
        return;
    }
    // make sure everything is valid
    assert(func.empty() && "parser didn't reject func within func");
    Value value = global.get(node.getName());
    assert(value.isFunc() &&
        "FuncResolver didn't run or didn't register function");
    llvm::Function* llvmFunc = value.getLLVMFunc();
    // create the entry block
    auto* entry = llvm::BasicBlock::Create(llvmCtx, "entry", llvmFunc);
    // add to current scope
    func.enter();
    func.setReturnType(node.getReturnType());
    // setup the parameters to be referenced as mutable variables
    builder.SetInsertPoint(entry);
    for (size_t i = 0; i < node.getNumParams(); ++i)
    {
        // get the vsl and llvm parameter representation
        const ParamNode& param = node.getParam(i);
        llvm::Argument* llvmParam = &*std::next(llvmFunc->arg_begin(), i);
        // load the parameter into a runtime variable
        llvm::Value* alloca = createEntryAlloca(llvmParam->getType(),
            param.getName());
        builder.CreateStore(llvmParam, alloca);
        // add that variable to function scope
        func.set(param.getName(), Value::getVar(param.getType(), alloca));
    }
    // generate the body
    node.getBody().accept(*this);
    // make sure the last block is terminated and not waiting to insert anymore
    //  instructions afterward
    auto* bb = builder.GetInsertBlock();
    if (bb && !bb->getTerminator())
    {
        if (node.getReturnType() == vslCtx.getVoidType())
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

void IREmitter::visitExtFunc(ExtFuncNode& node)
{
    assert(func.empty() && "parser didn't reject extfunc within func");
}

void IREmitter::visitParam(ParamNode& node)
{
}

void IREmitter::visitBlock(BlockNode& node)
{
    // create a new scope and visit all the statements inside
    func.enter();
    for (Node* statement : node.getStatements())
    {
        statement->accept(*this);
    }
    func.exit();
    result = Value::getNull();
}

void IREmitter::visitEmpty(EmptyNode& node)
{
    result = Value::getNull();
}

void IREmitter::visitVariable(VariableNode& node)
{
    // make sure the variable type is valid
    if (!node.getType()->isValid())
    {
        diag.print<Diag::INVALID_VAR_TYPE>(node);
        return;
    }
    llvm::Type* llvmType = node.getType()->toLLVMType(llvmCtx);
    llvm::Value* llvmValue;
    if (isGlobal())
    {
        // global variable
        // add a global constructor for this variable
        auto* funcType = llvm::FunctionType::get(builder.getVoidTy(),
            /*isVarArg=*/false);
        auto* ctorFunc = llvm::Function::Create(funcType,
            llvm::GlobalValue::InternalLinkage,
            llvm::Twine{ "vsl.init." } + node.getName());
        addGlobalCtor(ctorFunc);
        // ctorFunc is inserted after the global constructor here so that the
        //  global constructor doesn't become sandwiched in between two variable
        //  constructors which just looks bad
        module.getFunctionList().push_back(ctorFunc);
        // setup IRBuilder to emit code to initialize the global variable
        auto* insertBlock = llvm::BasicBlock::Create(llvmCtx, "entry",
            ctorFunc);
        builder.SetInsertPoint(insertBlock);
        builder.CreateRetVoid();
        // insert instructions before the return
        builder.SetInsertPoint(insertBlock->getTerminator());
        // add code to declare and initialize the global variable
        // determine the linkage type
        llvm::GlobalValue::LinkageTypes linkage =
            accessToLinkage(node.getAccess());
        // create a placeholder initializer value
        llvm::Constant* initializer = llvm::Constant::getNullValue(llvmType);
        // create the variable
        llvmValue = new llvm::GlobalVariable{ module, llvmType,
            /*isConstant=*/false, linkage, initializer, node.getName() };
        // add to global scope
        global.setVar(node.getName(), node.getType(), llvmValue);
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
    Value init = result;
    result = Value::getNull();
    // before initializing the variable, make sure that the initializer
    //  expression is actually valid
    if (!init)
    {
        return;
    }
    // match the var and init types
    if (node.getType() != init.getVSLType())
    {
        diag.print<Diag::MISMATCHING_VAR_TYPES>(node, *init.getVSLType());
        return;
    }
    // store the variable
    builder.CreateStore(init.getLLVMValue(), llvmValue);
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
    llvm::Value* cond;
    // make sure it's a bool
    if (result.getVSLType() == vslCtx.getBoolType())
    {
        cond = result.getLLVMValue();
    }
    else
    {
        diag.print<Diag::CANNOT_CONVERT>(node.getCondition(),
            *result.getVSLType(), *vslCtx.getBoolType());
        cond = builder.getFalse();
    }
    // create the necessary basic blocks
    llvm::Function* currentFunc = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(llvmCtx, "if.then");
    auto* endBlock = llvm::BasicBlock::Create(llvmCtx, "if.end");
    llvm::BasicBlock* elseBlock;
    if (node.hasElse())
    {
        elseBlock = llvm::BasicBlock::Create(llvmCtx, "if.else");
    }
    else
    {
        // branch to the end if the condition is false
        elseBlock = endBlock;
    }
    // create the branch instruction
    builder.CreateCondBr(cond, thenBlock, elseBlock);
    // generate then block
    func.enter();
    thenBlock->insertInto(currentFunc);
    builder.SetInsertPoint(thenBlock);
    node.getThen().accept(*this);
    branchTo(endBlock);
    func.exit();
    if (node.hasElse())
    {
        // generate else block
        func.enter();
        elseBlock->insertInto(currentFunc);
        builder.SetInsertPoint(elseBlock);
        node.getElse().accept(*this);
        branchTo(endBlock);
        func.exit();
    }
    // setup end block for other code after the IfNode if needed
    // this handles when both then and else cases have a return statement
    func.exit();
    if (!endBlock->use_empty())
    {
        // insert the end block
        endBlock->insertInto(currentFunc);
        builder.SetInsertPoint(endBlock);
    }
    else
    {
        // endBlock shouldn't've been created in the first place
        delete endBlock;
        // all code after the IfNode is unreachable
        builder.ClearInsertionPoint();
    }
    result = Value::getNull();
}

void IREmitter::visitReturn(ReturnNode& node)
{
    // special case: return without a value
    if (!node.hasValue())
    {
        builder.CreateRetVoid();
        return;
    }
    // validate the return value
    node.getValue().accept(*this);
    Value value = result;
    result = Value::getNull();
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
    // lookup the identifier within the function scope
    Value value = func.get(node.getName());
    if (!value)
    {
        // not in function scope so must be in the global scope
        value = global.get(node.getName());
    }
    // variables require a load instruction
    if (value.isVar())
    {
        const Type* type = value.getVSLType();
        result = Value::getExpr(type, builder.CreateLoad(value.getLLVMValue()));
    }
    // functions and expressions can be taken as is
    else if (value.isFunc() || value.isExpr())
    {
        result = value;
    }
    // emit an error if something bad happened
    else
    {
        diag.print<Diag::UNKNOWN_IDENT>(node);
        result = Value::getNull();
    }
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
    const Type* type = result.getVSLType();
    // choose the appropriate operator to generate code for
    switch (node.getOp())
    {
    case UnaryKind::NOT:
        // should only be valid on booleans
        if (type != vslCtx.getBoolType())
        {
            result = Value::getNull();
            break;
        }
        // fallthrough
    case UnaryKind::MINUS:
        genNeg(result);
        break;
    default:
        // should never happen
        result = Value::getNull();
    }
    if (!result)
    {
        diag.print<Diag::INVALID_UNARY>(node, *type);
    }
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
    node.getRhs().accept(*this);
    if (!result)
    {
        return;
    }
    Value rhs = result;
    result = Value::getNull();
    // make sure the types match
    if (lhs.getVSLType() == rhs.getVSLType())
    {
        // choose the appropriate operator to generate code for
        const Type* type = lhs.getVSLType();
        llvm::Value* lhsVal = lhs.getLLVMValue();
        llvm::Value* rhsVal = rhs.getLLVMValue();
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
    // setup blocks
    llvm::BasicBlock* currBlock = builder.GetInsertBlock();
    llvm::Function* currFunc = currBlock->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(llvmCtx, "ternary.then");
    auto* elseBlock = llvm::BasicBlock::Create(llvmCtx, "ternary.else");
    auto* contBlock = llvm::BasicBlock::Create(llvmCtx, "ternary.cont");
    // branch based on the condition
    builder.CreateCondBr(result.getLLVMValue(), thenBlock, elseBlock);
    // generate then
    // we get a reference to the current block after generating code because an
    //  expression can span multiple basic blocks, e.g. a ternary such as this
    thenBlock->insertInto(currFunc);
    builder.SetInsertPoint(thenBlock);
    node.getThen().accept(*this);
    Value thenCase = result;
    auto* thenEnd = builder.GetInsertBlock();
    branchTo(contBlock);
    // generate else
    elseBlock->insertInto(currFunc);
    builder.SetInsertPoint(elseBlock);
    // make sure thenCase is valid before generating elseCase
    if (!thenCase)
    {
        // something very bad happened
        delete contBlock;
        return;
    }
    node.getElse().accept(*this);
    Value elseCase = result;
    auto* elseEnd = builder.GetInsertBlock();
    branchTo(contBlock);
    // setup cont block for code that comes after
    contBlock->insertInto(currFunc);
    builder.SetInsertPoint(contBlock);
    if (!elseCase)
    {
        return;
    }
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
}

void IREmitter::visitCall(CallNode& node)
{
    // make sure the callee is an actual function
    node.getCallee().accept(*this);
    if (!result)
    {
        return;
    }
    if (!result.isFunc() || !result.getVSLType()->isFunctionType())
    {
        diag.print<Diag::NOT_A_FUNCTION>(node.getCallee(),
            *result.getVSLType());
        result = Value::getNull();
        return;
    }
    auto* calleeType = static_cast<const FunctionType*>(result.getVSLType());
    // make sure the right amount of arguments is used
    if (calleeType->getNumParams() != node.getNumArgs())
    {
        diag.print<Diag::MISMATCHING_ARG_COUNT>(node.getLoc(),
            node.getNumArgs(), calleeType->getNumParams());
        result = Value::getNull();
        return;
    }
    llvm::Value* func = result.getLLVMValue();
    std::vector<llvm::Value*> llvmArgs;
    // verify each argument
    for (size_t i = 0; i < calleeType->getNumParams(); ++i)
    {
        const Type* paramType = calleeType->getParamType(i);
        ArgNode& arg = node.getArg(i);
        arg.accept(*this);
        if (!result)
        {
            return;
        }
        // check that the types match
        if (result.getVSLType() == paramType)
        {
            llvmArgs.push_back(result.getLLVMValue());
        }
        else
        {
            diag.print<Diag::CANNOT_CONVERT>(arg.getValue(),
                *result.getVSLType(), *paramType);
        }
    }
    // create the call instruction if all args are fine with it
    if (calleeType->getNumParams() == llvmArgs.size())
    {
        result = Value::getExpr(calleeType->getReturnType(),
            builder.CreateCall(func, llvmArgs));
    }
    else
    {
        result = Value::getNull();
    }
}

void IREmitter::visitArg(ArgNode& node)
{
    node.getValue().accept(*this);
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
    rhs.accept(*this);
    if (!result)
    {
        return;
    }
    // make sure that the lhs is an identifier
    if (lhs.is(Node::IDENT))
    {
        // lookup the identifier
        auto& id = static_cast<IdentNode&>(lhs);
        if (Value value = func.get(id.getName()))
        {
            // make sure the types match up
            if (value.getVSLType() == result.getVSLType())
            {
                // finally, create the store instruction
                builder.CreateStore(result.getLLVMValue(),
                    value.getLLVMValue());
                return;
            }
            else
            {
                diag.print<Diag::CANNOT_CONVERT>(rhs, *result.getVSLType(),
                    *value.getVSLType());
            }
        }
        else
        {
            diag.print<Diag::UNKNOWN_IDENT>(id);
        }
    }
    else
    {
        diag.print<Diag::LHS_NOT_ASSIGNABLE>(lhs);
    }
    result = Value::getNull();
}

void IREmitter::genShortCircuit(BinaryNode& node)
{
    // generate code to calculate lhs
    ExprNode& lhs = node.getLhs();
    lhs.accept(*this);
    if (!result)
    {
        return;
    }
    Value cond1 = result;
    // of course, lhs has to be a bool for this to work
    if (cond1.getVSLType() != vslCtx.getBoolType())
    {
        diag.print<Diag::CANNOT_CONVERT>(lhs, *cond1.getVSLType(),
            *vslCtx.getBoolType());
        result = Value::getNull();
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
        builder.CreateCondBr(cond1.getLLVMValue(), longCheck, cont);
    }
    else // or
    {
        builder.CreateCondBr(cond1.getLLVMValue(), cont, longCheck);
    }
    // the long check is when the operation did not short circuit, and depends
    //  on the value of rhs to fully determine the result
    longCheck->insertInto(currFunc);
    builder.SetInsertPoint(longCheck);
    // generate code to calculate rhs
    ExprNode& rhs = node.getRhs();
    rhs.accept(*this);
    Value cond2 = result;
    // setup the cont block
    branchTo(cont);
    cont->insertInto(currFunc);
    builder.SetInsertPoint(cont);
    if (!result)
    {
        return;
    }
    // of course, rhs has to be a bool for this to work
    // the check happens later so that the cont block is neither a memory leak
    //  nor a dangling pointer, and we can safely insert other code afterwards
    if (result.getVSLType() != vslCtx.getBoolType())
    {
        diag.print<Diag::CANNOT_CONVERT>(rhs, *cond2.getVSLType(),
            *vslCtx.getBoolType());
        result = Value::getNull();
        return;
    }
    // create the phi instruction
    auto* phi = builder.CreatePHI(builder.getInt1Ty(), 2, name);
    // if the branch came from currBlock, then the operation short-circuited
    phi->addIncoming(builder.getInt1(node.getOp() == BinaryKind::OR),
        currBlock);
    // if it came from longCheck, then the result is determined by rhs
    phi->addIncoming(cond2.getLLVMValue(), longCheck);
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

void IREmitter::addGlobalCtor(llvm::Function* f)
{
    // get/create the global vsl initialization function
    auto* funcType = llvm::FunctionType::get(builder.getVoidTy(),
        /*isVarArg=*/false);
    llvm::BasicBlock* insertBlock;
    if (vslInit)
    {
        // function @vsl.init already exists
        assert(vslInit->getFunctionType() == funcType &&
            "Function @vsl.init has invalid type! Should be void ().");
        insertBlock = &vslInit->back();
    }
    else
    {
        // function @vsl.init doesn't already exist so create it now
        vslInit = llvm::Function::Create(funcType,
            llvm::GlobalValue::InternalLinkage, "vsl.init", &module);
        // terminate the function with a void return
        insertBlock = llvm::BasicBlock::Create(llvmCtx, "entry", vslInit);
        builder.SetInsertPoint(insertBlock);
        builder.CreateRetVoid();
        // create the @llvm.global_ctors intrinsic variable so that @vsl.init
        //  gets called at runtime
        /*
         * End result:
         * %0 = type { i32, void ()*, i8* }
         * @llvm.global_ctors = appending global [1 x %0]
         *     [%0 { i32 65535, void ()* @vsl.init, i8* null }]
         */
        assert(!module.getNamedGlobal("llvm.global_ctors") &&
            "@llvm.global_ctors already defined!");
        // create the required llvm::Type objects
        auto* priorityType = builder.getInt32Ty();
        auto* dataType = llvm::PointerType::getUnqual(builder.getInt8Ty());
        auto* ctorType = llvm::StructType::create("", priorityType,
            llvm::PointerType::getUnqual(funcType), dataType);
        auto* ctorArrayType = llvm::ArrayType::get(ctorType, 1);
        // create the initializer object
        auto* ctor = llvm::ConstantStruct::get(ctorType,
            builder.getInt32(65535), vslInit,
            llvm::ConstantPointerNull::get(dataType));
        auto* ctorArray = llvm::ConstantArray::get(ctorArrayType, ctor);
        // create the variable, which is automatically added to the module
        new llvm::GlobalVariable{ module, ctorArrayType, /*isConstant=*/false,
            llvm::GlobalValue::AppendingLinkage, ctorArray,
            "llvm.global_ctors" };
    }
    builder.SetInsertPoint(insertBlock->getTerminator());
    // call the given global ctor function
    assert(f->getFunctionType() == funcType &&
        "Invalid global ctor type! Should be void ().");
    builder.CreateCall(f);
}
