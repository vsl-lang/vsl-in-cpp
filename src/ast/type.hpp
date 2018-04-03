#ifndef TYPE_HPP
#define TYPE_HPP

class Type;
class SimpleType;
class NamedType;
class FunctionType;
class ClassType;

#include "ast/node.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"
#include <functional>
#include <vector>

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
        /** Void. */
        VOID,
        /** Boolean. */
        BOOL,
        /** Integer. */
        INT,
        /** Named. */
        NAMED,
        /** Function. */
        FUNCTION,
        /** Class. */
        CLASS
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
     * Creates a SimpleType.
     *
     * @param kind The kind of SimpleType this is.
     */
    SimpleType(Type::Kind kind);

private:
    virtual void printImpl(llvm::raw_ostream& os) const override;
};

/**
 * Represents a named (possibly unresolved) type.
 */
class NamedType : public Type
{
    friend class VSLContext;
public:
    bool operator==(const NamedType& type) const;
    llvm::StringRef getName() const;
    /**
     * Checks if the type has an underlying type. This is the type that will be
     * actually used when resolving it into an LLVM type.
     */
    bool hasUnderlyingType() const;
    const Type* getUnderlyingType() const;
    /**
     * Set the underlying type. This is a const method because the underlying
     * type needs to be a mutable field.
     *
     * @param type Underlying type.
     */
    void setUnderlyingType(const Type* type) const;

protected:
    /**
     * Creates a NamedType. Starts out with no underlying type.
     *
     * @param name Name of the type.
     */
    NamedType(llvm::StringRef name);

private:
    virtual void printImpl(llvm::raw_ostream& os) const override;
    void printUnderlyingType(llvm::raw_ostream& os) const;
    /** Name of the type. */
    llvm::StringRef name;
    /** The type being represented. */
    mutable const Type* underlyingType;
};

/**
 * Represents a VSL function.
 */
class FunctionType : public Type
{
    friend class VSLContext;
public:
    bool operator==(const FunctionType& type) const;
    size_t getNumParams() const;
    const Type* getParamType(size_t i) const;
    llvm::ArrayRef<const Type*> getParams() const;
    const Type* getReturnType() const;
    /**
     * Checks whether this is a constructor. If this is true, then getReturnType
     * should return the type of the object being created.
     */
    bool isCtor() const;
    bool isMethod() const;
    bool hasSelfType() const;
    /**
     * Gets the type of the `self` parameter. Returns null if this FunctionType
     * is not a ctor/method type.
     *
     * @returns The type of the `self` parameter.
     */
    const NamedType* getSelfType() const;
    void setSelfType(const NamedType* selfType);

protected:
    /**
     * Creates a FunctionType.
     *
     * @param params The parameters that the function takes.
     * @param returnType What type the function returns.
     * @param ctor Whether this is a constructor or not.
     */
    FunctionType(std::vector<const Type*> params, const Type* returnType,
        bool ctor = false);

private:
    virtual void printImpl(llvm::raw_ostream& os) const override;
    /** The parameters that the function takes. */
    std::vector<const Type*> params;
    /** What type the function returns. */
    const Type* returnType;
    /** Type of the self parameter. */
    const NamedType* selfType;
    /** Whether this is a constructor or not. */
    bool ctor;
};

/**
 * Represents a reference to an object of a specific class type.
 */
class ClassType : public Type
{
    friend class VSLContext;
public:
    /** Represents a field type. */
    struct Field
    {
        Field();
        Field(const Type* type, size_t index, Access access);
        bool isValid() const;
        operator bool() const;
        bool operator!() const;
        /** Type of the field. */
        const Type* type;
        /** Field index in the ClassNode. */
        size_t index;
        /** Access specifier. */
        Access access;
    };
    /**
     * Gets a field type, or null if it doesn't exist.
     *
     * @param name Name of the field to get.
     *
     * @returns Corresponding field type, or null if nonexistent.
     */
    Field getField(llvm::StringRef name) const;
    /**
     * Adds a new field.
     *
     * @param name Name of the field to create.
     * @param type Type of the field.
     * @param index Field index.
     * @param access Access specifier.
     *
     * @returns False if added successfully, true otherwise.
     */
    bool setField(llvm::StringRef name, const Type* type, size_t index,
        Access access);

protected:
    /**
     * Creates an opaque ClassType.
     */
    ClassType();

private:
    virtual void printImpl(llvm::raw_ostream& os) const override;
    /** Contained fields. */
    llvm::StringMap<Field> fieldTypes;
};

namespace std
{
/**
 * Template specialization for hashing {@link NamedType NamedTypes}.
 */
template<>
struct hash<NamedType>
{
    /**
     * Computes the hash value for a given NamedType. The underlying type is
     * never hashed because that has a mutable interface.
     *
     * @param type The NamedType to compute the hash value for.
     *
     * @returns The hash value for the given type.
     */
    size_t operator()(const NamedType& type) const
    {
        return llvm::hash_value(type.getName());
    }
};
/**
 * Template specialization for hashing {@link FunctionType FunctionTypes}.
 */
template<>
struct hash<FunctionType>
{
    /**
     * Computes the hash value for a given FunctionType.
     *
     * @param ft The FunctionType to compute the hash value for.
     *
     * @returns The hash value for ft.
     */
    size_t operator()(const FunctionType& type) const
    {
        return llvm::hash_combine(type.isCtor(), type.getSelfType(),
            type.getReturnType(), type.getParams());
    }
};
} // end namespace std

#endif // TYPE_HPP
