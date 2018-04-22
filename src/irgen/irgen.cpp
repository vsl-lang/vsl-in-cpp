#include "irgen/irgen.hpp"
#include "irgen/passes/funcResolver/funcResolver.hpp"
#include "irgen/passes/typeResolver/typeResolver.hpp"
#include "irgen/passes/irEmitter/irEmitter.hpp"
#include "llvm/IR/Verifier.h"

IRGen::IRGen(VSLContext& vslCtx, Diag& diag, llvm::Module& module)
    : vslCtx{ vslCtx }, diag{ diag }, module{ module },
    converter{ vslCtx, module.getContext() }
{
}

void IRGen::run()
{
    // resolve type declarations
    TypeResolver typeResolver{ vslCtx, converter, module };
    typeResolver.visitAST(vslCtx.getGlobals());
    // resolve global functions
    FuncResolver funcResolver{ vslCtx, diag, global, converter, module };
    funcResolver.visitAST(vslCtx.getGlobals());
    // emit code for global functions
    IREmitter irEmitter{ vslCtx, diag, func, global, converter, module };
    irEmitter.visitAST(vslCtx.getGlobals());
    // the module should be valid after all this
    std::string s;
    llvm::raw_string_ostream sos{ s };
    if (llvm::verifyModule(module, &sos))
    {
        diag.print<Diag::LLVM_MODULE_ERROR>(std::move(sos.str()));
    }
}
