#ifndef IRGEN_HPP
#define IRGEN_HPP

#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "irgen/scope/funcScope.hpp"
#include "irgen/scope/globalScope.hpp"
#include "irgen/typeConverter/typeConverter.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Module.h"

/**
 * Manages all the AST passes to emit LLVM IR.
 */
class IRGen
{
public:
    /**
     * Creates an IREmitter object.
     *
     * @param vslCtx Context object for VSL stuff.
     * @param diag Diagnostics manager.
     * @param module Where to emit LLVM IR.
     */
    IRGen(VSLContext& vslCtx, Diag& diag, llvm::Module& module);
    /**
     * Runs all the AST passes, converting the AST stored in the VSLContext to
     * LLVM IR in the Module.
     */
    void run();

private:
    /** Context object for VSL stuff. */
    VSLContext& vslCtx;
    /** Diagnostics manager. */
    Diag& diag;
    /** Where to emit LLVM IR. */
    llvm::Module& module;
    /** Function scope manager. */
    FuncScope func;
    /** Global scope manager. */
    GlobalScope global;
    /** VSL to LLVM type converter. */
    TypeConverter converter;
};

#endif // IRGEN_HPP
