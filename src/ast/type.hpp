#ifndef TYPE_HPP
#define TYPE_HPP

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include <functional>
#include <string>
#include <vector>

/**
 * Represents a VSL type.
 */
class Type
{
    friend class VSLContext;
    friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os,
        const Type& type);
public:
    /**
     * Resolves the Type into an LLVM type.
     *
     * @param context The context object that owns the LLVM type.
     *
     * @returns An LLVM representation of the Type.
     */
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& context) const = 0;
    /**
     * Verifies whether the Type is valid and can be stored in a variable, i.e.,
     * not Void or Error.
     *
     * @returns True if this type is valid, false otherwise.
     */
    bool isValid() const;
    /**
     * Checks whether this type is a FunctionType.
     *
     * @returns True if this type is a FunctionType, false otherwise.
     */
    bool isFunctionType() const;
    /**
     * Prints a Type.
     *
     * @param os Stream to print to.
     */
    void print(llvm::raw_ostream& os) const;

protected:
    /**
     * Specifies the kind of Type object being represented. Keep in mind that
     * one Type subclass can be represented by multiple Kinds.
     */
    enum Kind
    {
        /** Error. */
        ERROR,
        /** Boolean. */
        BOOL,
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
     * Gets the Type::Kind in a more human-readable format.
     *
     * @param k The Kind to get the name for.
     *
     * @returns The string representation of the Type kind.
     */
    static const char* getKindName(Kind k);
    /**
     * Gets the kind of Type this is.
     *
     * @returns The kind of Type this is.
     */
    Kind getKind() const;

private:
    /**
     * Virtual print implementation.
     *
     * @param os Stream to print to.
     */
    virtual void printImpl(llvm::raw_ostream& os) const = 0;
    /** Represents what kind of Type object this is. */
    Kind kind;
};

/**
 * Simple type with no extra data.
 */
class SimpleType : public Type
{
    friend class VSLContext;
public:
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& context) const override;

protected:
    /**
     * Creates a SimpleType object.
     *
     * @param kind The kind of SimpleType this is.
     */
    SimpleType(Type::Kind kind);

private:
    virtual void printImpl(llvm::raw_ostream& os) const override;
};

/**
 * Represents a VSL function.
 */
class FunctionType : public Type
{
    friend class VSLContext;
    friend struct std::hash<FunctionType>;
public:
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& context) const override;
    /**
     * Tests for equality.
     *
     * @param ft The other FunctionType to compare with.
     *
     * @returns True if they are equal, false otherwise.
     */
    bool operator==(const FunctionType& ft) const;
    size_t getNumParams() const;
    const Type* getParamType(size_t i) const;
    llvm::ArrayRef<const Type*> getParams() const;
    const Type* getReturnType() const;

protected:
    /**
     * Creates a FunctionType object.
     *
     * @param params The parameters that the function takes.
     * @param returnType What type the function returns.
     */
    FunctionType(std::vector<const Type*> params, const Type* returnType);

private:
    virtual void printImpl(llvm::raw_ostream& os) const override;
    /** The parameters that the function takes. */
    std::vector<const Type*> params;
    /** What type the function returns. */
    const Type* returnType;
};

namespace std
{
/**
 * Template specialization for hashing FunctionTypes.
 */
template<>
struct hash<FunctionType>
{
    /**
     * Computes the hash value for a given FunctionType.
     *
     * @param ft The FunctionType to compute the hash value for.
     *
     * @returns The hash value for `ft`.
     */
    size_t operator()(const FunctionType& ft) const
    {
        size_t value = 17;
        value = value * 31 + std::hash<const Type*>{}(ft.returnType);
        for (const Type* param : ft.params)
        {
            value = value * 31 + std::hash<const Type*>{}(param);
        }
        return value;
    }
};
} // end namespace std

#endif // TYPE_HPP
