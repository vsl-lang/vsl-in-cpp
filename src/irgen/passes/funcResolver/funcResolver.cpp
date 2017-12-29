#include "irgen/passes/funcResolver/funcResolver.hpp"

FuncResolver::FuncResolver(Diag& diag, GlobalScope& global,
    llvm::Module& module)
    : diag{ diag }, global{ global }, module{ module },
    llvmCtx{ module.getContext() }
{
}

void FuncResolver::visitFunction(FunctionNode& node)
{
    // create the llvm function
    auto* ft = static_cast<llvm::FunctionType*>(
        node.getFunctionType()->toLLVMType(llvmCtx));
    auto* f = llvm::Function::Create(ft, llvm::GlobalValue::ExternalLinkage,
        node.getName(), &module);
    // add to global scope, emitting an error if already defined
    if (global.setFunc(node.getName(), node.getFunctionType(), f))
    {
        diag.print<Diag::FUNC_ALREADY_DEFINED>(node);
        // iremitter will skip this node later
        node.setAlreadyDefined();
    }
}
