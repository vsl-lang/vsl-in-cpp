#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <iostream>
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"

/**
 * Generates object code from an llvm::Module.
 */
class CodeGen
{
public:
    /**
     * Creates a CodeGen object.
     *
     * @param module The module to compile.
     * @param errors The stream to print errors to.
     */
    CodeGen(llvm::Module& module, std::ostream& errors = std::cerr);
    /**
     * Compiles the module.
     *
     * @param output The stream to write the result to.
     */
    void compile(llvm::raw_pwrite_stream& output);
    /**
     * Checks if the lexer has encountered an error yet. In this case, a warning
     * would also count as an error.
     *
     * @returns True if the lexer encountered an error, false otherwise.
     */
    bool hasError() const;
    /**
     * Runs a series of optimization passes on the module.
     *
     * @param optLevel The optimization level to use.
     */
    void optimize(int optLevel);

private:
    /**
     * The module to compile.
     */
    llvm::Module& module;
    /**
     * Handles all the module-level transformations, such as compilation.
     */
    llvm::legacy::PassManager pm;
    /**
     * Handles all the function-level transformations, such as optimization.
     */
    llvm::legacy::FunctionPassManager fpm;
    /**
     * The stream to print errors to.
     */
    std::ostream& errors;
    /**
     * True if the lexer encountered an error, otherwise false.
     */
    bool errored;
};

#endif // CODEGEN_HPP
