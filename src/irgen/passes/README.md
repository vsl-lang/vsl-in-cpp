# Passes
Each folder here is an AST pass that implements the NodeVisitor interface.

List of passes:
1. [TypeResolver](typeResolver/typeResolver.hpp): Generates class types in the global scope.
2. [FuncResolver](funcResolver/funcResolver.hpp): Processes free functions and methods/ctors in the global scope so they can be called ahead of their definition.
3. [IREmitter](irEmitter/irEmitter.hpp): Does type checking and LLVM IR generation of everything.

More info can be found in the doxygen documentation.
