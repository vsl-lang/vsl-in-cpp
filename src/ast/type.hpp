#ifndef TYPE_HPP
#define TYPE_HPP

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/raw_ostream.h"
#include <functional>
#include <vector>

class Type;

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const Type& type);

/**
 * Represents a VSL type. Derived Types are stored in a VSLContext separate from
 * eachother, therefore not needing a virtual destructor. This may change in the
 * future as the type system becomes more complex.
 */
class Type
{
    friend class VSLContext;
public:
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
    Kind getKind() const;
    bool is(Kind kind) const;
    bool isNot(Kind kind) const;
    bool isFunctionType() const;
    /**
     * Verifies whether the Type is valid and can be stored in a variable, i.e.,
     * not Void or Error.
     *
     * @returns True if this type is valid, false otherwise.
     */
    bool isValid() const;
    /**
     * Prints a Type.
     *
     * @param os Stream to print to.
     */
    void print(llvm::raw_ostream& os) const;

protected:
    /**
     * Creates a Type object.
     *
     * @param kind The kind of Type being created.
     */
    Type(Kind kind);
    /**
     * Gets the name of a Type::Kind.
     *
     * @param k The Kind to get the name for.
     *
     * @returns The string representation of the Type kind.
     */
    static const char* getKindName(Kind k);

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
 * Simple builtin type with no extra data.
 */
class SimpleType : public Type
{
    friend class VSLContext;

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
        value = value * 31 + hash<const Type*>{}(ft.returnType);
        for (const Type* param : ft.params)
        {
            value = value * 31 + hash<const Type*>{}(param);
        }
        return value;
    }
};
} // end namespace std

#endif // TYPE_HPP
