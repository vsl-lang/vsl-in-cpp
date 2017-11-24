#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "irgen/vslContext.hpp"
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
     * @param vslContext The VSLContext object to be used.
     * @param module The module to compile.
     */
    CodeGen(VSLContext& vslContext, llvm::Module& module);
    /**
     * Compiles the module.
     *
     * @param output The stream to write the result to.
     */
    void compile(llvm::raw_pwrite_stream& output);
    /**
     * Runs a series of optimization passes on the module.
     */
    void optimize();

private:
    /** Reference to the VSLContext object. */
    VSLContext& vslContext;
    /** The module to compile. */
    llvm::Module& module;
    /** Handles all the module-level transformations, such as compilation. */
    llvm::legacy::PassManager pm;
    /** Handles all the function-level transformations, such as optimization. */
    llvm::legacy::FunctionPassManager fpm;
};

#endif // CODEGEN_HPP
