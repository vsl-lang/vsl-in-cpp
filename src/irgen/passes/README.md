# Passes
Each folder here is an AST pass that implements the NodeVisitor interface.

List of passes:
1. [FuncResolver](funcResolver/funcResolver.hpp): Processes free functions in the global scope so they can be called ahead of their definition.
2. [IREmitter](irEmitter/irEmitter.hpp): Does type checking and LLVM IR generation.

More info can be found in the doxygen documentation.
