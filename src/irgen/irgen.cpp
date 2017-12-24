#include "irgen/irgen.hpp"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include <limits>

IRGen::IRGen(VSLContext& vslContext, Diag& diag, llvm::Module& module)
    : vslContext{ vslContext }, diag{ diag }, module{ module },
    context{ module.getContext() }, builder{ context },
    allocaInsertPoint{ nullptr }, result{ nullptr }
{
}

void IRGen::visitEmpty(EmptyNode& node)
{
    result = nullptr;
}

void IRGen::visitBlock(BlockNode& node)
{
    // create a new scope and visit all the statements inside
    scopeTree.enter();
    for (Node* statement : node.getStatements())
    {
        statement->accept(*this);
    }
    scopeTree.exit();
    result = nullptr;
}

void IRGen::visitIf(IfNode& node)
{
    // make sure that it's not in the global scope.
    if (scopeTree.isGlobal())
    {
        diag.print<Diag::TOPLEVEL_CTRL_FLOW>(node.getLoc());
    }
    // setup the condition
    scopeTree.enter();
    node.getCondition()->accept(*this);
    const Type* type = node.getCondition()->getType();
    llvm::Value* cond;
    if (type == vslContext.getBoolType())
    {
        // take the value as is
        cond = result;
    }
    else if (type == vslContext.getIntType())
    {
        // convert the Int to a Bool
        cond = builder.CreateICmpNE(result,
            llvm::ConstantInt::get(context, llvm::APInt{ 32, 0 }));
    }
    else
    {
        // error, assume false
        diag.print<Diag::CANNOT_CONVERT>(*node.getCondition(),
            *vslContext.getBoolType());
        cond = llvm::ConstantInt::getFalse(context);
    }
    // create the necessary basic blocks
    llvm::Function* currentFunc = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(context, "if.then");
    auto* elseBlock = llvm::BasicBlock::Create(context, "if.else");
    auto* endBlock = llvm::BasicBlock::Create(context, "if.end");
    // create the branch instruction
    builder.CreateCondBr(cond, thenBlock, elseBlock);
    // generate then block
    scopeTree.enter();
    thenBlock->insertInto(currentFunc);
    builder.SetInsertPoint(thenBlock);
    node.getThen()->accept(*this);
    branchTo(endBlock);
    scopeTree.exit();
    // generate else block
    scopeTree.enter();
    elseBlock->insertInto(currentFunc);
    builder.SetInsertPoint(elseBlock);
    node.getElse()->accept(*this);
    branchTo(endBlock);
    scopeTree.exit();
    // setup end block for other code after the IfNode if needed
    // this handles when both then and else cases have a return statement
    scopeTree.exit();
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

void IRGen::visitVariable(VariableNode& node)
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
        node.getType()->toLLVMType(context), node.getName());
    // add to current scope
    if (!scopeTree.set(node.getName(), { node.getType(), alloca }))
    {
        diag.print<Diag::VAR_ALREADY_DEFINED>(node);
        alloca->eraseFromParent();
        return;
    }
    // store the variable
    builder.CreateStore(initializer, alloca);
}

void IRGen::visitFunction(FunctionNode& node)
{
    // create the llvm function and the entry block
    auto* ft = static_cast<llvm::FunctionType*>(
        node.getFunctionType()->toLLVMType(context));
    auto* f = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage,
        node.getName(), &module);
    auto* entry = llvm::BasicBlock::Create(context, "entry", f);
    // add to current scope
    scopeTree.set(node.getName(), { node.getFunctionType(), f });
    scopeTree.enter(node.getReturnType());
    // setup the parameters to be referenced as mutable variables
    builder.SetInsertPoint(entry);
    for (size_t i = 0; i < node.getNumParams(); ++i)
    {
        const ParamNode& param = *node.getParam(i);
        llvm::Value* alloca = createEntryAlloca(ft->getParamType(i),
            param.getName());
        builder.CreateStore(&f->arg_begin()[i], alloca);
        scopeTree.set(param.getName(), { param.getType(), alloca });
    }
    // generate the body
    node.getBody()->accept(*this);
    // make sure the last block is terminated and not waiting to insert anymore
    //  instructions afterward
    auto* bb = builder.GetInsertBlock();
    if (bb && !bb->getTerminator())
    {
        if (node.getReturnType() == vslContext.getVoidType())
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
    scopeTree.exit();
    // erase the alloca point because nobody needs to see it
    if (allocaInsertPoint)
    {
        allocaInsertPoint->eraseFromParent();
        allocaInsertPoint = nullptr;
    }
    result = nullptr;
    // make sure that the function is valid, else internal error
    std::string s;
    llvm::raw_string_ostream sos{ s };
    if (llvm::verifyFunction(*f, &sos))
    {
        diag.print<Diag::LLVM_FUNC_ERROR>();
    }
}

void IRGen::visitParam(ParamNode& node)
{
}

void IRGen::visitReturn(ReturnNode& node)
{
    // special case: return without a value
    if (!node.getValue())
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
        if (type == vslContext.getVoidType())
        {
            diag.print<Diag::CANT_RETURN_VOID_VALUE>(node);
        }
        else if (type != scopeTree.getReturnType())
        {
            diag.print<Diag::RETVAL_MISMATCHES_RETTYPE>(*node.getValue(),
                *scopeTree.getReturnType());
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

void IRGen::visitIdent(IdentNode& node)
{
    // lookup the name in the current scope
    Scope::Item i = scopeTree.get(node.getName());
    if (i.type == nullptr || i.value == nullptr)
    {
        diag.print<Diag::UNKNOWN_IDENT>(node);
        node.setType(vslContext.getErrorType());
        result = nullptr;
        return;
    }
    node.setType(i.type);
    // an identifier can reference either a variable or a function
    if (node.getType()->isFunctionType())
    {
        result = i.value;
    }
    else
    {
        result = builder.CreateLoad(i.value);
    }
}

void IRGen::visitLiteral(LiteralNode& node)
{
    // create an LLVM integer
    unsigned width = node.getValue().getBitWidth();
    switch (width)
    {
    case 1:
        node.setType(vslContext.getBoolType());
        break;
    case 32:
        node.setType(vslContext.getIntType());
        break;
    default:
        // should never happen
        diag.print<Diag::INVALID_INT_WIDTH>(node);
        node.setType(vslContext.getErrorType());
    }
    result = llvm::ConstantInt::get(context, node.getValue());
}

void IRGen::visitUnary(UnaryNode& node)
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

void IRGen::visitBinary(BinaryNode& node)
{
    // handle assignment operator as a special case
    if (node.getOp() == TokenKind::ASSIGN)
    {
        genAssign(node);
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
            node.setType(vslContext.getBoolType());
            genEQ(type, lhs, rhs);
            break;
        case TokenKind::NOT_EQUAL:
            node.setType(vslContext.getBoolType());
            genNE(type, lhs, rhs);
            break;
        case TokenKind::GREATER:
            node.setType(vslContext.getBoolType());
            genGT(type, lhs, rhs);
            break;
        case TokenKind::GREATER_EQUAL:
            node.setType(vslContext.getBoolType());
            genGE(type, lhs, rhs);
            break;
        case TokenKind::LESS:
            node.setType(vslContext.getBoolType());
            genLT(type, lhs, rhs);
            break;
        case TokenKind::LESS_EQUAL:
            node.setType(vslContext.getBoolType());
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

void IRGen::visitCall(CallNode& node)
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
    node.setType(vslContext.getErrorType());
    result = nullptr;
}

void IRGen::visitArg(ArgNode& node)
{
    node.getValue()->accept(*this);
}

void IRGen::genAssign(BinaryNode& node)
{
    ExprNode& lhs = *node.getLhs();
    ExprNode& rhs = *node.getRhs();
    rhs.accept(*this);
    llvm::Value* rightValue = result;
    node.setType(vslContext.getVoidType());
    result = nullptr;
    // make sure that the lhs is an identifier
    if (lhs.is(Node::IDENT))
    {
        // lookup the identifier
        auto& id = static_cast<IdentNode&>(lhs);
        Scope::Item i = scopeTree.get(id.getName());
        if (i.type && i.value)
        {
            // make sure the types match up
            if (i.type == rhs.getType())
            {
                // finally, create the store instruction
                builder.CreateStore(rightValue, i.value);
                return;
            }
            else
            {
                diag.print<Diag::CANNOT_CONVERT>(rhs, *i.type);
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

void IRGen::genNeg(const Type* type, llvm::Value* value)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateNeg(value, "neg") : nullptr;
}

void IRGen::genAdd(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateAdd(lhs, rhs, "add") : nullptr;
}

void IRGen::genSub(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateSub(lhs, rhs, "sub") : nullptr;
}

void IRGen::genMul(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateMul(lhs, rhs, "mul") : nullptr;
}

void IRGen::genDiv(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateSDiv(lhs, rhs, "sdiv") : nullptr;
}

void IRGen::genMod(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateSRem(lhs, rhs, "srem") : nullptr;
}

void IRGen::genEQ(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType() ||
            type == vslContext.getBoolType()) ?
        builder.CreateICmpEQ(lhs, rhs, "cmp") : nullptr;
}

void IRGen::genNE(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType() ||
            type == vslContext.getBoolType()) ?
        builder.CreateICmpNE(lhs, rhs, "cmp") : nullptr;
}

void IRGen::genGT(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateICmpSGT(lhs, rhs, "cmp") : nullptr;
}

void IRGen::genGE(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateICmpSGE(lhs, rhs, "cmp") : nullptr;
}

void IRGen::genLT(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateICmpSLT(lhs, rhs, "cmp") : nullptr;
}

void IRGen::genLE(const Type* type, llvm::Value* lhs, llvm::Value* rhs)
{
    result = (type == vslContext.getIntType()) ?
        builder.CreateICmpSLE(lhs, rhs, "cmp") : nullptr;
}

llvm::AllocaInst* IRGen::createEntryAlloca(llvm::Type* type,
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

llvm::BranchInst* IRGen::branchTo(llvm::BasicBlock* target)
{
    if (builder.GetInsertBlock() && !builder.GetInsertBlock()->getTerminator())
    {
        return builder.CreateBr(target);
    }
    return nullptr;
}
