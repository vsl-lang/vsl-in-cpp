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
    llvmCtx{ module.getContext() }, builder{ llvmCtx },
    allocaInsertPoint{ nullptr }, result{ nullptr }
{
}

void IREmitter::visitEmpty(EmptyNode& node)
{
    result = nullptr;
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
    result = nullptr;
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
    node.getCondition()->accept(*this);
    const Type* type = node.getCondition()->getType();
    llvm::Value* cond;
    // make sure it's a bool
    if (type == vslCtx.getBoolType())
    {
        cond = result;
    }
    else
    {
        diag.print<Diag::CANNOT_CONVERT>(*node.getCondition(),
            *vslCtx.getBoolType());
        cond = builder.getFalse();
    }
    // create the necessary basic blocks
    llvm::Function* currentFunc = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(llvmCtx, "if.then");
    auto* elseBlock = llvm::BasicBlock::Create(llvmCtx, "if.else");
    auto* endBlock = llvm::BasicBlock::Create(llvmCtx, "if.end");
    // create the branch instruction
    builder.CreateCondBr(cond, thenBlock, elseBlock);
    // generate then block
    func.enter();
    thenBlock->insertInto(currentFunc);
    builder.SetInsertPoint(thenBlock);
    node.getThen()->accept(*this);
    branchTo(endBlock);
    func.exit();
    // generate else block
    func.enter();
    elseBlock->insertInto(currentFunc);
    builder.SetInsertPoint(elseBlock);
    node.getElse()->accept(*this);
    branchTo(endBlock);
    func.exit();
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
    result = nullptr;
}

void IREmitter::visitVariable(VariableNode& node)
{
    ExprNode& init = *node.getInit();
    init.accept(*this);
    llvm::Value* initializer = result;
    result = nullptr;
    // do type checking
    if (!node.getType()->isValid())
    {
        diag.print<Diag::INVALID_VAR_TYPE>(node);
        return;
    }
    if (node.getType() != init.getType())
    {
        diag.print<Diag::MISMATCHING_VAR_TYPES>(node);
        return;
    }
    // allocate the variable
    llvm::AllocaInst* alloca = createEntryAlloca(
        node.getType()->toLLVMType(llvmCtx), node.getName());
    // add to current scope
    if (func.set(node.getName(), node.getType(), alloca))
    {
        diag.print<Diag::VAR_ALREADY_DEFINED>(node);
        alloca->eraseFromParent();
        return;
    }
    // store the variable
    builder.CreateStore(initializer, alloca);
}

void IREmitter::visitFunction(FunctionNode& node)
{
    if (node.isAlreadyDefined())
    {
        // probably flagged by FuncResolver
        return;
    }
    if (!func.empty())
    {
        // funcception!
        diag.print<Diag::FUNC_IN_FUNC>(node);
        return;
    }
    // create the llvm function and the entry block
    llvm::Function* f = global.getFunc(node.getName()).getLLVMFunc();
    assert(f && "FuncResolver didn't run or didn't register function");
    auto* entry = llvm::BasicBlock::Create(llvmCtx, "entry", f);
    // add to current scope
    func.enter();
    func.setReturnType(node.getReturnType());
    // setup the parameters to be referenced as mutable variables
    builder.SetInsertPoint(entry);
    for (size_t i = 0; i < node.getNumParams(); ++i)
    {
        const ParamNode& param = *node.getParam(i);
        llvm::Argument* llvmParam = &*std::next(f->arg_begin(), i);
        llvm::Value* alloca = createEntryAlloca(llvmParam->getType(),
            param.getName());
        builder.CreateStore(llvmParam, alloca);
        func.set(param.getName(), param.getType(), alloca);
    }
    // generate the body
    node.getBody()->accept(*this);
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
    result = nullptr;
}

void IREmitter::visitParam(ParamNode& node)
{
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
    node.getValue()->accept(*this);
    const Type* type = node.getValue()->getType();
    llvm::Value* retVal = result;
    result = nullptr;
    if (retVal)
    {
        if (type == vslCtx.getVoidType())
        {
            diag.print<Diag::CANT_RETURN_VOID_VALUE>(node);
        }
        else if (type != func.getReturnType())
        {
            diag.print<Diag::RETVAL_MISMATCHES_RETTYPE>(*node.getValue(),
                *func.getReturnType());
        }
        else
        {
            // nothing bad happened yay
            builder.CreateRet(retVal);
            return;
        }
    }
    // errors in the return value expression create an unreachable instruction
    //  instead of the usual ret
    builder.CreateUnreachable();
}

void IREmitter::visitIdent(IdentNode& node)
{
    // load a variable
    if (VarItem var = func.get(node.getName()))
    {
        node.setType(var.getVSLType());
        result = builder.CreateLoad(var.getLLVMValue());
    }
    // call a function
    else if (FuncItem fn = global.getFunc(node.getName()))
    {
        node.setType(fn.getVSLType());
        result = fn.getLLVMFunc();
    }
    // or emit an error
    else
    {
        diag.print<Diag::UNKNOWN_IDENT>(node);
        node.setType(vslCtx.getErrorType());
        result = nullptr;
    }
}

void IREmitter::visitLiteral(LiteralNode& node)
{
    // create an LLVM integer
    unsigned width = node.getValue().getBitWidth();
    switch (width)
    {
    case 1:
        node.setType(vslCtx.getBoolType());
        break;
    case 32:
        node.setType(vslCtx.getIntType());
        break;
    default:
        // should never happen
        diag.print<Diag::INVALID_INT_WIDTH>(node);
        node.setType(vslCtx.getErrorType());
    }
    result = llvm::ConstantInt::get(llvmCtx, node.getValue());
}

void IREmitter::visitUnary(UnaryNode& node)
{
    // verify the contained expression
    node.getExpr()->accept(*this);
    const Type* type = node.getExpr()->getType();
    node.setType(type);
    llvm::Value* value = result;
    result = nullptr;
    // choose the appropriate operator to generate code for
    switch (node.getOp())
    {
    case TokenKind::MINUS:
    case TokenKind::NOT:
        genNeg(type, value);
        break;
    default:
        // should never happen
        ;
    }
    if (result == nullptr)
    {
        diag.print<Diag::INVALID_UNARY>(node);
    }
}

void IREmitter::visitBinary(BinaryNode& node)
{
    // special case: variable assignment
    if (node.getOp() == TokenKind::ASSIGN)
    {
        genAssign(node);
        return;
    }
    // special case: short-circuiting boolean operations
    if (node.getOp() == TokenKind::AND || node.getOp() == TokenKind::OR)
    {
        genShortCircuit(node);
        return;
    }
    // verify the left and right expressions
    node.getLhs()->accept(*this);
    llvm::Value* lhs = result;
    node.getRhs()->accept(*this);
    llvm::Value* rhs = result;
    result = nullptr;
    // make sure the types match
    if (node.getLhs()->getType() == node.getRhs()->getType())
    {
        // choose the appropriate operator to generate code for
        const Type* type = node.getLhs()->getType();
        switch (node.getOp())
        {
        case TokenKind::PLUS:
            node.setType(node.getLhs()->getType());
            genAdd(type, lhs, rhs);
            break;
        case TokenKind::MINUS:
            node.setType(node.getLhs()->getType());
            genSub(type, lhs, rhs);
            break;
        case TokenKind::STAR:
            node.setType(node.getLhs()->getType());
            genMul(type, lhs, rhs);
            break;
        case TokenKind::SLASH:
            node.setType(node.getLhs()->getType());
            genDiv(type, lhs, rhs);
            break;
        case TokenKind::PERCENT:
            node.setType(node.getLhs()->getType());
            genMod(type, lhs, rhs);
            break;
        case TokenKind::EQUAL:
            node.setType(vslCtx.getBoolType());
            genEQ(type, lhs, rhs);
            break;
        case TokenKind::NOT_EQUAL:
            node.setType(vslCtx.getBoolType());
            genNE(type, lhs, rhs);
            break;
        case TokenKind::GREATER:
            node.setType(vslCtx.getBoolType());
            genGT(type, lhs, rhs);
            break;
        case TokenKind::GREATER_EQUAL:
            node.setType(vslCtx.getBoolType());
            genGE(type, lhs, rhs);
            break;
        case TokenKind::LESS:
            node.setType(vslCtx.getBoolType());
            genLT(type, lhs, rhs);
            break;
        case TokenKind::LESS_EQUAL:
            node.setType(vslCtx.getBoolType());
            genLE(type, lhs, rhs);
            break;
        default:
            // should never happen
            node.setType(node.getLhs()->getType());
        }
    }
    else
    {
        // types mismatch, assume lhs' type
        node.setType(node.getLhs()->getType());
    }
    if (result == nullptr)
    {
        diag.print<Diag::INVALID_BINARY>(node);
    }
}

void IREmitter::visitTernary(TernaryNode& node)
{
    // generate condition and make sure its a bool
    node.getCondition()->accept(*this);
    llvm::Value* condition = result;
    result = nullptr;
    if (node.getCondition()->getType() != vslCtx.getBoolType())
    {
        diag.print<Diag::CANNOT_CONVERT>(*node.getCondition(),
            *vslCtx.getBoolType());
        return;
    }
    // setup blocks
    llvm::BasicBlock* currBlock = builder.GetInsertBlock();
    llvm::Function* currFunc = currBlock->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(llvmCtx, "ternary.then");
    auto* elseBlock = llvm::BasicBlock::Create(llvmCtx, "ternary.else");
    auto* contBlock = llvm::BasicBlock::Create(llvmCtx, "ternary.cont");
    // branch
    builder.CreateCondBr(condition, thenBlock, elseBlock);
    // generate then
    // we get a reference to the current block after generating code because an
    //  expression can span multiple basic blocks, e.g. a ternary such as this
    thenBlock->insertInto(currFunc);
    builder.SetInsertPoint(thenBlock);
    node.getThen()->accept(*this);
    llvm::Value* thenCase = result;
    auto* thenEnd = builder.GetInsertBlock();
    branchTo(contBlock);
    // generate else
    elseBlock->insertInto(currFunc);
    builder.SetInsertPoint(elseBlock);
    node.getElse()->accept(*this);
    llvm::Value* elseCase = result;
    auto* elseEnd = builder.GetInsertBlock();
    branchTo(contBlock);
    // setup cont block for code that comes after
    contBlock->insertInto(currFunc);
    builder.SetInsertPoint(contBlock);
    // do type checking to make sure everything's fine
    node.setType(node.getThen()->getType());
    if (node.getThen()->getType() != node.getElse()->getType())
    {
        diag.print<Diag::TERNARY_TYPE_MISMATCH>(node);
        result = nullptr;
        return;
    }
    // bring it all together with a phi node
    auto* phi = builder.CreatePHI(thenCase->getType(), 2, "ternary.phi");
    phi->addIncoming(thenCase, thenEnd);
    phi->addIncoming(elseCase, elseEnd);
    result = phi;
}

void IREmitter::visitCall(CallNode& node)
{
    // make sure the callee is an actual function
    node.getCallee()->accept(*this);
    const auto* calleeType =
        static_cast<const FunctionType*>(node.getCallee()->getType());
    if (!calleeType->isFunctionType())
    {
        diag.print<Diag::NOT_A_FUNCTION>(*node.getCallee());
    }
    // make sure the right amount of arguments is used
    else if (calleeType->getNumParams() != node.getNumArgs())
    {
        diag.print<Diag::MISMATCHING_ARG_COUNT>(node.getLoc(),
            node.getNumArgs(), calleeType->getNumParams());
    }
    else
    {
        llvm::Value* func = result;
        std::vector<llvm::Value*> llvmArgs;
        // verify each argument
        for (size_t i = 0; i < calleeType->getNumParams(); ++i)
        {
            const Type* paramType = calleeType->getParamType(i);
            ArgNode& arg = *node.getArg(i);
            arg.accept(*this);
            // check that the types match
            if (arg.getValue()->getType() != paramType)
            {
                diag.print<Diag::CANNOT_CONVERT>(*arg.getValue(), *paramType);
            }
            else
            {
                llvmArgs.push_back(result);
            }
        }
        // create the call instruction if all args were successfully validated
        if (calleeType->getNumParams() == llvmArgs.size())
        {
            node.setType(calleeType->getReturnType());
            result = builder.CreateCall(func, llvmArgs);
            return;
        }
    }
    // if any sort of error occured, then this happens
    node.setType(vslCtx.getErrorType());
    result = nullptr;
}

void IREmitter::visitArg(ArgNode& node)
{
    node.getValue()->accept(*this);
}

void IREmitter::genAssign(BinaryNode& node)
{
    ExprNode& lhs = *node.getLhs();
    ExprNode& rhs = *node.getRhs();
    rhs.accept(*this);
    llvm::Value* rightValue = result;
    node.setType(vslCtx.getVoidType());
    result = nullptr;
    // make sure that the lhs is an identifier
    if (lhs.is(Node::IDENT))
    {
        // lookup the identifier
        auto& id = static_cast<IdentNode&>(lhs);
        if (VarItem var = func.get(id.getName()))
        {
            // make sure the types match up
            if (var.getVSLType() == rhs.getType())
            {
                // finally, create the store instruction
                builder.CreateStore(rightValue, var.getLLVMValue());
                return;
            }
            else
            {
                diag.print<Diag::CANNOT_CONVERT>(rhs, *var.getVSLType());
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
}
void IREmitter::genShortCircuit(BinaryNode& node)
{
    // boolean and/or obviously return bool sooo
    node.setType(vslCtx.getBoolType());
    // generate code to calculate lhs
    ExprNode& lhs = *node.getLhs();
    lhs.accept(*this);
    llvm::Value* cond1 = result;
    // of course, lhs has to be a bool for this to work
    if (lhs.getType() != vslCtx.getBoolType())
    {
        diag.print<Diag::CANNOT_CONVERT>(lhs, *vslCtx.getBoolType());
        result = nullptr;
        return;
    }
    // helper variables so i don't have to type as much
    auto* currBlock = builder.GetInsertBlock();
    llvm::Function* currFunc = currBlock->getParent();
    // setup blocks
    llvm::Twine name = (node.getOp() == TokenKind::AND) ? "and" : "or";
    auto* longCheck = llvm::BasicBlock::Create(llvmCtx, name + ".long");
    auto* cont = llvm::BasicBlock::Create(llvmCtx, name + ".cont");
    // create the branch
    if (node.getOp() == TokenKind::AND)
    {
        builder.CreateCondBr(cond1, longCheck, cont);
    }
    else // or
    {
        builder.CreateCondBr(cond1, cont, longCheck);
    }
    // the long check is when the operation did not short circuit, and depends
    //  on the value of rhs to fully determine the result
    longCheck->insertInto(currFunc);
    builder.SetInsertPoint(longCheck);
    // generate code to calculate rhs
    ExprNode& rhs = *node.getRhs();
    rhs.accept(*this);
    llvm::Value* cond2 = result;
    // setup the cont block
    branchTo(cont);
    cont->insertInto(currFunc);
    builder.SetInsertPoint(cont);
    // of course, rhs has to be a bool for this to work
    // the check happens later so that the cont block is neither a memory leak
    //  nor a dangling pointer, and we can safely insert other code afterwards
    if (rhs.getType() != vslCtx.getBoolType())
    {
        diag.print<Diag::CANNOT_CONVERT>(rhs, *vslCtx.getBoolType());
        result = nullptr;
        return;
    }
    // create the phi instruction
    auto* phi = builder.CreatePHI(builder.getInt1Ty(), 2, name);
    // if the branch came from currBlock, then the operation short-circuited
    phi->addIncoming(builder.getInt1(node.getOp() == TokenKind::OR),
        currBlock);
    // if it came from longCheck, then the result is determined by rhs
    phi->addIncoming(cond2, longCheck);
    result = phi;
}

void IREmitter::genNeg(const Type* type, llvm::Value* value)
{
    result = (type == vslCtx.getIntType() || type == vslCtx.getBoolType()) ?
        builder.CreateNeg(value, "neg") : nullptr;
}

void IREmitter::genAdd(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        builder.CreateAdd(lhs, rhs, "add") : nullptr;
}

void IREmitter::genSub(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        builder.CreateSub(lhs, rhs, "sub") : nullptr;
}

void IREmitter::genMul(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        builder.CreateMul(lhs, rhs, "mul") : nullptr;
}

void IREmitter::genDiv(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        builder.CreateSDiv(lhs, rhs, "sdiv") : nullptr;
}

void IREmitter::genMod(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        builder.CreateSRem(lhs, rhs, "srem") : nullptr;
}

void IREmitter::genEQ(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType() ||
            type == vslCtx.getBoolType()) ?
        builder.CreateICmpEQ(lhs, rhs, "cmp") : nullptr;
}

void IREmitter::genNE(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType() ||
            type == vslCtx.getBoolType()) ?
        builder.CreateICmpNE(lhs, rhs, "cmp") : nullptr;
}

void IREmitter::genGT(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        builder.CreateICmpSGT(lhs, rhs, "cmp") : nullptr;
}

void IREmitter::genGE(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        builder.CreateICmpSGE(lhs, rhs, "cmp") : nullptr;
}

void IREmitter::genLT(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        builder.CreateICmpSLT(lhs, rhs, "cmp") : nullptr;
}

void IREmitter::genLE(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslCtx.getIntType()) ?
        builder.CreateICmpSLE(lhs, rhs, "cmp") : nullptr;
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
