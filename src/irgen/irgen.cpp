#include "irgen/irgen.hpp"
#include "irgen/passes/funcResolver/funcResolver.hpp"
#include "irgen/passes/irEmitter/irEmitter.hpp"
#include "llvm/IR/Verifier.h"

IRGen::IRGen(VSLContext& vslContext, Diag& diag, llvm::Module& module)
    : vslContext{ vslContext }, diag{ diag }, module{ module }
{
}

void IRGen::run(llvm::MutableArrayRef<Node*> statements)
{
    FuncResolver funcResolver{ diag, global, module };
    funcResolver.visitStatements(statements);
    IREmitter emitter{ vslContext, diag, func, global, module };
    emitter.visitStatements(statements);
    // the module should be valid after all this
    std::string s;
    llvm::raw_string_ostream sos{ s };
    if (llvm::verifyModule(module, &sos))
    {
        diag.print<Diag::LLVM_MODULE_ERROR>(std::move(sos.str()));
    }
}
