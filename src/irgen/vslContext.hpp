#ifndef VSLCONTEXT_HPP
#define VSLCONTEXT_HPP

#include "ast/type.hpp"
#include <unordered_set>

class VSLContext
{
public:
    /**
     * Constructs a VSLContext.
     */
    VSLContext();
    /**
     * Gets a SimpleType.
     *
     * @param k The kind of SimpleType to get.
     *
     * @returns The corresponding SimpleType.
     */
    const SimpleType* getSimpleType(Type::Kind k);
    /**
     * Gets or constructs a FunctionType.
     *
     * @param params The list of parameters.
     * @param returnType The return type.
     *
     * @returns A FunctionType using the given parameters.
     */
    const FunctionType* getFunctionType(std::vector<const Type*> params,
        const Type* returnType);

private:
    /** Placeholder for any type errors. */
    SimpleType errorType;
    /** Represents the Bool type. */
    SimpleType boolType;
    /** Represents the Int type. */
    SimpleType intType;
    /** Represents the Void type. */
    SimpleType voidType;
    /** Contains all the FunctionTypes. */
    std::unordered_set<FunctionType> functionTypes;
};

#endif // VSLCONTEXT_HPP
