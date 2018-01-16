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
    auto* f = createFunc(node.getAccessMod(), node.getFuncType(),
        node.getName());
    // add to global scope
    global.setFunc(node.getName(), node.getFuncType(), f);
}

void FuncResolver::visitExtFunc(ExtFuncNode& node)
{
    if (global.getFunc(node.getName()))
    {
        diag.print<Diag::FUNC_ALREADY_DEFINED>(node);
    }
    // create the llvm function
    auto* f = createFunc(node.getAccessMod(), node.getFuncType(),
        node.getAlias());
    // add to global scope
    global.setFunc(node.getName(), node.getFuncType(), f);
}

llvm::Function* FuncResolver::createFunc(AccessMod access,
    const FunctionType* ft, const llvm::Twine& name)
{
    assert((access == AccessMod::PUBLIC || access == AccessMod::PRIVATE) &&
        "functions must have access specifiers");
    llvm::GlobalValue::LinkageTypes linkage;
    switch (access)
    {
    case AccessMod::PUBLIC:
        linkage = llvm::GlobalValue::ExternalLinkage;
        break;
    default: // should never happen
    case AccessMod::PRIVATE:
        linkage = llvm::GlobalValue::InternalLinkage;
        break;
    }
    auto* llvmft = static_cast<llvm::FunctionType*>(ft->toLLVMType(llvmCtx));
    return llvm::Function::Create(llvmft, linkage, name, &module);
}
