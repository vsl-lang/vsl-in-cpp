#ifndef FUNCRESOLVER_HPP
#define FUNCRESOLVER_HPP

#include "ast/node.hpp"
#include "ast/nodeVisitor.hpp"
#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "irgen/scope/globalScope.hpp"
#include "irgen/typeConverter/typeConverter.hpp"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/Module.h"

/**
 * Resolves functions in the global scope to allow calling them without stuff
 * like C's forward declarations.
 */
class FuncResolver : public NodeVisitor
{
public:
    /**
     * Creates a FuncResolver.
     *
     * @param vslCtx Context object for VSL stuff.
     * @param diag Diagnostics manager.
     * @param global Used for entering in VSL functions.
     * @param converter VSL to LLVM type converter.
     * @param module Used for entering in LLVM functions.
     */
    FuncResolver(VSLContext& vslCtx, Diag& diag, GlobalScope& global,
        TypeConverter& converter, llvm::Module& module);
    virtual ~FuncResolver() override = default;
    virtual void visitFunction(FunctionNode& node) override;
    virtual void visitExtFunc(ExtFuncNode& node) override;
    virtual void visitClass(ClassNode& node) override;
    virtual void visitCtor(CtorNode& node) override;
    virtual void visitMethod(MethodNode& node) override;

private:
    /**
     * Verifies a function's name to make sure it's not being named after a type
     * or a pre-existing function. Prints the necessary error diagnostics and
     * returns true if this is the case.
     *
     * @param node Function to verify.
     *
     * @returns True if the function's name is already taken, false otherwise.
     */
    bool verifyFuncName(const FuncInterfaceNode& node) const;
    /**
     * Creates an LLVM function.
     *
     * @param access VSL access specifier. Must not be NONE.
     * @param ft VSL function type.
     * @param name Function name.
     */
    llvm::Function* createFunc(Access access, const FunctionType* ft,
        const llvm::Twine& name = "");
    /**
     * Declares a class destructor. This creates the LLVM function and enters it
     * into the GlobalScope.
     *
     * @param node Class to declare the destructor for.
     */
    void declareDtor(const ClassNode& node);
    /** Context object for VSL stuff. */
    VSLContext& vslCtx;
    /** Diagnostics manager. */
    Diag& diag;
    /** Used for entering in VSL functions. */
    GlobalScope& global;
    /** VSL to LLVM type converter. */
    TypeConverter& converter;
    /** Used for entering in LLVM functions. */
    llvm::Module& module;
    /** Context object for LLVM stuff. */
    llvm::LLVMContext& llvmCtx;
};

#endif // FUNCRESOLVER_HPP
