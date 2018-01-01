#ifndef IRGEN_HPP
#define IRGEN_HPP

#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "irgen/scope/funcScope.hpp"
#include "irgen/scope/globalScope.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Module.h"

class IRGen
{
public:
    /**
     * Creates an IREmitter object.
     *
     * @param vslContext Context object for VSL stuff.
     * @param diag Diagnostics manager.
     * @param module Where to emit LLVM IR.
     */
    IRGen(VSLContext& vslContext, Diag& diag, llvm::Module& module);
    /**
     * Runs all the AST passes, converting it to LLVM IR in the module.
     *
     * @param statements The statements to visit.
     */
    void run(llvm::ArrayRef<Node*> statements);

private:
    /** Context object for VSL stuff. */
    VSLContext& vslContext;
    /** Diagnostics manager. */
    Diag& diag;
    /** Where to emit LLVM IR. */
    llvm::Module& module;
    /** Function scope manager. */
    FuncScope func;
    /** Global scope manager. */
    GlobalScope global;
};

#endif // IRGEN_HPP
