#ifndef TYPECONVERTER_HPP
#define TYPECONVERTER_HPP

#include "ast/vslContext.hpp"
#include "irgen/scope/globalScope.hpp"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include <unordered_map>

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
     * Converts a type. This returns the LLVM opaque type if no type exists that
     * could represent the given VSL type.
     *
     * @param type VSL type to convert.
     *
     * @returns An equivalent LLVM type, or the opaque type if nonexistent.
     */
    llvm::Type* convert(const Type* type) const;
    llvm::Type* convert(const SimpleType* type) const;
    llvm::Type* convert(const NamedType* type) const;
    llvm::FunctionType* convert(const FunctionType* type) const;
    /**
     * Converts a class type. This should be a pointer to a reference-counted
     * struct, which holds the reference count and the class struct type.
     *
     * @returns A reference type.
     */
    llvm::PointerType* convert(const ClassType* type) const;
    /**
     * Adds a class type. This internally creates the reference-counted LLVM
     * struct type for this type. The resulting type is a pointer to allow easy
     * copying between function scopes.
     *
     * @param name Name of the class. Used to create the reference-counted type.
     * @param vslType VSL type of class. Must not already be added.
     * @param structType LLVM struct type.
     */
    void addClassType(llvm::StringRef name, const ClassType* vslType,
        llvm::StructType* structType);

private:
    /**
     * Gets the LLVM opaque type. Used to represent types that don't exist.
     *
     * @returns The LLVM opaque type.
     */
    llvm::StructType* getOpaqueType() const;
    /** LLVM context object. */
    llvm::LLVMContext& llvmCtx;
    /** Maps VSL class types to LLVM references. */
    std::unordered_map<const ClassType*, llvm::PointerType*> classes;
};

#endif // TYPECONVERTER_HPP
