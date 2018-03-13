#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "diag/diag.hpp"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Target/TargetMachine.h"

/**
 * Generates native object code from an llvm::Module.
 */
class CodeGen
{
public:
    /**
     * Creates a CodeGen object.
     *
     * @param diag Diagnostics manager.
     * @param module The module to compile.
     */
    CodeGen(Diag& diag, llvm::Module& module);
    /**
     * Configures the module with target data.
     */
    void configure();
    /**
     * Compiles the module. Configure must be run before this.
     *
     * @param output The stream to write the result to.
     */
    void compile(llvm::raw_pwrite_stream& output);
    /**
     * Runs a series of optimization passes on the module.
     */
    void optimize();

private:
    /** Diagnostics manager. */
    Diag& diag;
    /** The module to compile. */
    llvm::Module& module;
    /** Handles all the module-level transformations, such as compilation. */
    llvm::legacy::PassManager pm;
    /** Handles all the function-level transformations, such as optimization. */
    llvm::legacy::FunctionPassManager fpm;
    /** Machine to generate code for. */
    llvm::TargetMachine* machine;
};

#endif // CODEGEN_HPP
