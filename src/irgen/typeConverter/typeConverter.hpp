#ifndef TYPECONVERTER_HPP
#define TYPECONVERTER_HPP

#include "irgen/scope/globalScope.hpp"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"

/**
 * Converts a VSL type to an LLVM type.
 */
class TypeConverter
{
public:
    /**
     * Creates a TypeConverter.
     *
     * @param llvmCtx LLVM context object.
     */
    TypeConverter(llvm::LLVMContext& llvmCtx);
    /**
     * Converts a type.
     *
     * @param type VSL type to convert.
     *
     * @returns An equivalent LLVM type.
     */
    llvm::Type* convert(const Type* type) const;
    llvm::Type* convert(const SimpleType* type) const;
    llvm::FunctionType* convert(const FunctionType* type) const;

private:
    /** LLVM context object. */
    llvm::LLVMContext& llvmCtx;
};

#endif // TYPECONVERTER_HPP
