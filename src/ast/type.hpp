#ifndef TYPE_HPP
#define TYPE_HPP

class Type;
class SimpleType;
class UnresolvedType;
class FunctionType;
class ClassType;

class VSLContext;

#include "ast/node.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"
#include <functional>
#include <vector>

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const Type& type);

/**
 * Represents a VSL type.
 */
class Type
{
    friend class VSLContext;
public:
    virtual ~Type() = 0;
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
        /** Unresolved. */
        UNRESOLVED,
        /** Function. */
        FUNCTION,
        /** Class. */
        CLASS
    };
    Kind getKind() const;
    bool is(Kind kind) const;
    bool isNot(Kind kind) const;
    const UnresolvedType* asUnresolvedType() const;
    const FunctionType* asFunctionType() const;
    const ClassType* asClassType() const;
    /**
     * Verifies whether the Type is valid and can be stored in a variable, i.e.,
     * not Void or Error.
     *
     * @returns True if this type is valid, false otherwise.
     */
    bool isValid() const;
    /**
     * Compares this type's most basic form with the given type's most basic
     * form. Used in type checking.
     *
     * @param type Type to compare. Must not be null.
     * @param vslCtx Required for some types.
     *
     * @returns Whether the types match up or not.
     */
    bool matches(const Type* type, VSLContext& vslCtx) const;
    /**
     * Gets the most basic form of this type. Used for type checking.
     *
     * @param vslCtx Required for some Types.
     *
     * @returns The most basic form of this type.
     */
    const Type* resolve(VSLContext& vslCtx) const;
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
    /**
     * Virtual resolve implementation.
     */
    virtual const Type* resolveImpl(VSLContext& vslCtx) const;

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
public:
    virtual ~SimpleType() override = default;

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
 * Represents a named unresolved type.
 */
class UnresolvedType : public Type
{
    friend class VSLContext;
public:
    virtual ~UnresolvedType() override = default;
    bool operator==(const UnresolvedType& type) const;
    llvm::StringRef getName() const;

protected:
    /**
     * Creates an UnresolvedType.
     *
     * @param name Name of the type.
     */
    UnresolvedType(llvm::StringRef name);
    virtual const Type* resolveImpl(VSLContext& vslCtx) const override;

private:
    virtual void printImpl(llvm::raw_ostream& os) const override;
    /** Name of the type. */
    llvm::StringRef name;
    /** Actual type. Cached when resolve is called. */
    mutable const Type* actualType;
};

/**
 * Represents a VSL function.
 */
class FunctionType : public Type
{
    friend class VSLContext;
public:
    virtual ~FunctionType() override = default;
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
    const ClassType* getSelfType() const;
    void setSelfType(const ClassType* selfType);

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
    const ClassType* selfType;
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

    virtual ~ClassType() override = default;
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
    /**
     * Gets the name of the class.
     *
     * @returns The name of the class.
     */
    llvm::StringRef getName() const;

protected:
    /**
     * Creates an empty ClassType.
     *
     * @param name Name of the class.
     */
    ClassType(llvm::StringRef name);

private:
    virtual void printImpl(llvm::raw_ostream& os) const override;
    /** Contained fields. */
    llvm::StringMap<Field> fieldTypes;
    /** Name of the class. */
    llvm::StringRef name;
};

namespace std
{
/**
 * Template specialization for hashing {@link UnresolvedType UnresolvedTypes}.
 */
template<>
struct hash<UnresolvedType>
{
    /**
     * Computes the hash value for a given UnresolvedType.
     *
     * @param type The UnresolvedType to compute the hash value for.
     *
     * @returns The hash value for the type.
     */
    size_t operator()(const UnresolvedType& type) const
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
     * @param type The FunctionType to compute the hash value for.
     *
     * @returns The hash value for the type.
     */
    size_t operator()(const FunctionType& type) const
    {
        // TODO: we should hash based on the resolved forms of these types
        return llvm::hash_combine(type.isCtor(), type.getSelfType(),
            type.getReturnType(), type.getParams());
    }
};
} // end namespace std

#endif // TYPE_HPP
