#ifndef TYPE_HPP
#define TYPE_HPP

#include <memory>
#include <string>
#include <vector>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"

/**
 * Represents a VSL type.
 */
class Type
{
public:
    /**
     * Specifies the kind of Type object being represented. Keep in mind that
     * one Type subclass can be represented by multiple Kinds.
     */
    enum Kind
    {
        /** Error. */
        ERROR,
        /** Integer. */
        INT,
        /** Function. */
        FUNCTION,
        /** Void. */
        VOID
    };
    /**
     * Creates a Type object.
     *
     * @param kind The kind of Type being created.
     */
    Type(Kind kind);
    /**
     * Destroys a Type object.
     */
    virtual ~Type() = 0;
    /**
     * Gets the string representation of a given Kind.
     *
     * @param kind The kind to get the string representation for.
     *
     * @returns The string representation of a given Kind
     */
    static const char* kindToString(Type::Kind kind);
    /**
     * Deep clones a Type object.
     *
     * @returns An identical clone of the Type.
     */
    virtual std::unique_ptr<Type> clone() const = 0;
    /**
     * Converts the Type into a string.
     *
     * @returns A string representation of the Type.
     */
    virtual std::string toString() const = 0;
    /**
     * Resolves the Type into an LLVM type.
     *
     * @param context The context object that owns the LLVM type.
     *
     * @returns An LLVM representation of the Type.
     */
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& context) const = 0;
    /** Represents what kind of Type object this is. */
    Kind kind;
};

/**
 * Simple type with no extra data.
 */
class SimpleType : public Type
{
public:
    /**
     * Creates a SimpleType object.
     *
     * @param kind The kind of SimpleType this is.
     */
    SimpleType(Type::Kind kind);
    /**
     * Destroys a SimpleType object.
     */
    virtual ~SimpleType() override = default;
    virtual std::unique_ptr<Type> clone() const override;
    virtual std::string toString() const override;
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& context) const override;
};

/**
 * Represents a VSL function.
 */
class FunctionType : public Type
{
public:
    /**
     * Creates a FunctionType object.
     *
     * @param params The parameters that the function takes.
     * @param returnType What type the function returns.
     */
    FunctionType(std::vector<std::unique_ptr<Type>> params,
        std::unique_ptr<Type> returnType);
    /**
     * Destroys a FunctionType object.
     */
    virtual ~FunctionType() override = default;
    virtual std::unique_ptr<Type> clone() const override;
    virtual std::string toString() const override;
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& context) const override;
    /** The parameters that the function takes. */
    std::vector<std::unique_ptr<Type>> params;
    /** What type the function returns. */
    std::unique_ptr<Type> returnType;
};

#endif // TYPE_HPP
