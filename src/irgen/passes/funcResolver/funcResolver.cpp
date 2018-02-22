#include "irgen/passes/funcResolver/funcResolver.hpp"

FuncResolver::FuncResolver(Diag& diag, GlobalScope& global,
    llvm::Module& module)
    : diag{ diag }, global{ global }, module{ module },
    llvmCtx{ module.getContext() }
{
}

void FuncResolver::visitFunction(FunctionNode& node)
{
    if (global.get(node.getName()))
    {
        diag.print<Diag::FUNC_ALREADY_DEFINED>(node);
        node.setAlreadyDefined(true);
    }
    // create the llvm function
    auto* f = createFunc(node.getAccess(), node.getFuncType(), node.getName());
    // add to global scope
    global.setFunc(node.getName(), node.getFuncType(), f);
}

void FuncResolver::visitExtFunc(ExtFuncNode& node)
{
    if (global.get(node.getName()))
    {
        diag.print<Diag::FUNC_ALREADY_DEFINED>(node);
    }
    // create the llvm function using the alias name
    auto* f = createFunc(node.getAccess(), node.getFuncType(), node.getAlias());
    // add to global scope using the function name
    // the function is referred to by its real name in VSL and by its alias name
    //  in the LLVM IR or everywhere else that isn't VSL.
    global.setFunc(node.getName(), node.getFuncType(), f);
}

llvm::Function* FuncResolver::createFunc(Access access, const FunctionType* ft,
    const llvm::Twine& name)
{
    assert(access != Access::NONE && "functions must have access specifiers");
    // get the LLVM linkage and function type
    llvm::GlobalValue::LinkageTypes linkage = accessToLinkage(access);
    auto* llvmft = static_cast<llvm::FunctionType*>(ft->toLLVMType(llvmCtx));
    // create the LLVM function
    return llvm::Function::Create(llvmft, linkage, name, &module);
}
