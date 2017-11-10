#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <iostream>
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"

/**
 * Generates native object code from an llvm::Module.
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
     * Checks if an error has been encountered yet.
     *
     * @returns True if an error was encountered, false otherwise.
     */
    bool hasError() const;
    /**
     * Runs a series of optimization passes on the module.
     */
    void optimize();

private:
    /** The module to compile. */
    llvm::Module& module;
    /** Handles all the module-level transformations, such as compilation. */
    llvm::legacy::PassManager pm;
    /** Handles all the function-level transformations, such as optimization. */
    llvm::legacy::FunctionPassManager fpm;
    /** The stream to print errors to. */
    std::ostream& errors;
    /** True if an error was encountered, otherwise false. */
    bool errored;
};

#endif // CODEGEN_HPP
