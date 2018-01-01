#include "irgen/passes/funcResolver/funcResolver.hpp"

FuncResolver::FuncResolver(Diag& diag, GlobalScope& global,
    llvm::Module& module)
    : diag{ diag }, global{ global }, module{ module },
    llvmCtx{ module.getContext() }
{
}

void FuncResolver::visitFunction(FunctionNode& node)
{
    if (global.getFunc(node.getName()))
    {
        diag.print<Diag::FUNC_ALREADY_DEFINED>(node);
        node.setAlreadyDefined(true);
    }
    // create the llvm function
    auto* ft = static_cast<llvm::FunctionType*>(
        node.getFunctionType()->toLLVMType(llvmCtx));
    auto* f = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage,
        node.getName(), &module);
    // add to global scope
    global.setFunc(node.getName(), node.getFunctionType(), f);
}

void FuncResolver::visitExtFunc(ExtFuncNode& node)
{
    if (global.getFunc(node.getName()))
    {
        diag.print<Diag::FUNC_ALREADY_DEFINED>(node);
    }
    // create the llvm function
    auto* ft = static_cast<llvm::FunctionType*>(
        node.getFunctionType()->toLLVMType(llvmCtx));
    auto* f = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage,
        node.getAlias(), &module);
    // add to global scope
    global.setFunc(node.getName(), node.getFunctionType(), f);
}
