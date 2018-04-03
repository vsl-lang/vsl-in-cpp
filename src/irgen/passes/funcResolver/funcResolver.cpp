#include "irgen/passes/funcResolver/funcResolver.hpp"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include <cassert>

FuncResolver::FuncResolver(VSLContext& vslCtx, Diag& diag, GlobalScope& global,
    TypeConverter& converter, llvm::Module& module)
    : vslCtx{ vslCtx }, diag{ diag }, global{ global }, converter{ converter },
    module{ module }, llvmCtx{ module.getContext() }
{
}

void FuncResolver::visitFunction(FunctionNode& node)
{
    // creates the function declaration
    if (verifyFuncName(node))
    {
        // this function already exists! flag it so the IREmitter won't emit it
        node.setAlreadyDefined(true);
        return;
    }
    // create the llvm function
    const FunctionType* ft = vslCtx.getFunctionType(node);
    llvm::Function* llvmFunc = createFunc(node.getAccess(), ft, node.getName());
    // add to global scope
    global.setFunc(node.getName(), ft, llvmFunc);
}

void FuncResolver::visitExtFunc(ExtFuncNode& node)
{
    // creates the function declaration
    if (verifyFuncName(node))
    {
        return;
    }
    // create the llvm function using the alias name
    const FunctionType* ft = vslCtx.getFunctionType(node);
    llvm::Function* llvmFunc = createFunc(node.getAccess(), ft,
        node.getAlias());
    // add to global scope using the function name
    // the function is referred to by its real name in VSL and by its alias name
    //  in the LLVM IR or everywhere else that isn't VSL.
    global.setFunc(node.getName(), ft, llvmFunc);
}

void FuncResolver::visitClass(ClassNode& node)
{
    // declare constructor
    if (node.hasCtor())
    {
        node.getCtor().accept(*this);
    }
    // declare methods
    for (MethodNode* method : node.getMethods())
    {
        method->accept(*this);
    }
    // declare destructor
    declareDtor(node);
}

void FuncResolver::visitCtor(CtorNode& node)
{
    // creates the class constructor declaration
    ClassNode& parent = node.getParent();
    // create the function type
    const FunctionType* ft = vslCtx.getFunctionType(node);
    // create the llvm function
    llvm::Function* llvmFunc = createFunc(
        mergeAccess(parent.getAccess(), node.getAccess()), ft,
        parent.getName() + ".ctor");
    // register everything in the global scope
    global.setCtor(parent.getType(), ft, llvmFunc, node.getAccess());
}

void FuncResolver::visitMethod(MethodNode& node)
{
    // creates the method declaration
    ClassNode& parent = node.getParent();
    // create the function type
    const FunctionType* ft = vslCtx.getFunctionType(node);
    // create the llvm function
    llvm::Function* llvmFunc = createFunc(
        mergeAccess(parent.getAccess(), node.getAccess()), ft,
        llvm::Twine{ parent.getName() } + llvm::Twine{ '.' } + node.getName());
    // register the method in the global scope
    global.setMethod(parent.getType(), node.getName(), ft, llvmFunc,
        node.getAccess());
}

bool FuncResolver::verifyFuncName(const FuncInterfaceNode& node) const
{
    if (global.get(node.getName()))
    {
        diag.print<Diag::FUNC_ALREADY_DEFINED>(node);
        return true;
    }
    if (vslCtx.hasNamedType(node.getName()))
    {
        diag.print<Diag::FUNC_NAMED_AFTER_TYPE>(node);
        return true;
    }
    return false;
}

llvm::Function* FuncResolver::createFunc(Access access, const FunctionType* ft,
    const llvm::Twine& name)
{
    assert(access != Access::NONE && "functions must have access specifiers");
    // get the LLVM linkage and function type
    llvm::GlobalValue::LinkageTypes linkage = accessToLinkage(access);
    llvm::FunctionType* llvmft = converter.convert(ft);
    // create the LLVM function
    return llvm::Function::Create(llvmft, linkage, name, &module);
}

void FuncResolver::declareDtor(const ClassNode& node)
{
    auto* llvmft = llvm::FunctionType::get(llvm::Type::getVoidTy(llvmCtx),
        { converter.convert(node.getType()) }, /*isVarArg=*/false);
    auto* llvmFunc = llvm::Function::Create(llvmft,
        accessToLinkage(node.getAccess()), node.getName() + ".dtor", &module);
    global.setDtor(node.getType(), llvmFunc);
}
