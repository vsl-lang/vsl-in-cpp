#ifndef FUNCRESOLVER_HPP
#define FUNCRESOLVER_HPP

#include "ast/node.hpp"
#include "ast/nodevisitor.hpp"
#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "irgen/scope/globalScope.hpp"
#include "llvm/IR/LLVMContext.h"
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
     * @param module Used for entering in LLVM functions.
     */
    FuncResolver(VSLContext& vslCtx, Diag& diag, GlobalScope& global,
        llvm::Module& module);
    virtual ~FuncResolver() override = default;
    virtual void visitFunction(FunctionNode& node) override;
    virtual void visitExtFunc(ExtFuncNode& node) override;

private:
    /**
     * Creates an LLVM function.
     *
     * @param access VSL access specifier. Must not be NONE.
     * @param ft VSL function type.
     * @param name Function name.
     */
    llvm::Function* createFunc(Access access, const FunctionType* ft,
        const llvm::Twine& name = "");
    /** Context object for VSL stuff. */
    VSLContext& vslCtx;
    /** Diagnostics manager. */
    Diag& diag;
    /** Used for entering in VSL functions. */
    GlobalScope& global;
    /** Used for entering in LLVM functions. */
    llvm::Module& module;
    /** Context object for LLVM stuff. */
    llvm::LLVMContext& llvmCtx;
};

#endif // FUNCRESOLVER_HPP
