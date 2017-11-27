#ifndef VSLCONTEXT_HPP
#define VSLCONTEXT_HPP

#include "ast/type.hpp"
#include "lexer/location.hpp"
#include "llvm/Support/raw_ostream.h"
#include <unordered_set>

/**
 * Context object that owns/manages VSL AST-specific data that shouldn't be kept
 * in the AST itself.
 */
class VSLContext
{
public:
    /**
     * Constructs a VSLContext.
     *
     * @param errors The stream to print errors to.
     */
    VSLContext(llvm::raw_ostream& errors = llvm::errs());
    /**
     * Gets the Bool type.
     *
     * @returns The Bool type.
     */
    const SimpleType* getBoolType() const;
    /**
     * Gets the Int type.
     *
     * @returns The Int type.
     */
    const SimpleType* getIntType() const;
    /**
     * Gets the Void type.
     *
     * @returns The Void type.
     */
    const SimpleType* getVoidType() const;
    /**
     * Gets the Error type.
     *
     * @returns The Error type.
     */
    const SimpleType* getErrorType() const;
    /**
     * Gets a SimpleType.
     *
     * @param k The kind of SimpleType to get.
     *
     * @returns The corresponding SimpleType.
     */
    const SimpleType* getSimpleType(Type::Kind k) const;
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
    /**
     * Prints an error, allowing the caller to add on any other useful info.
     *
     * @returns A stream to insert a diagnostic message.
     */
    llvm::raw_ostream& error();
    /**
     * Prints an error, allowing the caller to add on any other useful info. The
     * location is prefixed in bold to let the user know where it occured.
     *
     * @param location Where the error occured.
     *
     * @returns A stream to insert a diagnostic message.
     */
    llvm::raw_ostream& error(Location location);
    /**
     * Gets the amount of errors encountered.
     *
     * @returns The amount of erros encountered.
     */
    size_t getErrorCount() const;

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
    /** Stream to print errors to. */
    llvm::raw_ostream& errors;
    /** Amount of errors encountered. */
    size_t errorCount;
};

#endif // VSLCONTEXT_HPP
