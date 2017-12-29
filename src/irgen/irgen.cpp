#include "irgen/irgen.hpp"
#include "irgen/passes/funcResolver/funcResolver.hpp"
#include "irgen/passes/irEmitter/irEmitter.hpp"

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
}
