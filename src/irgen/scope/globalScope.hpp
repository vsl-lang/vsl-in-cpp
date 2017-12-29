#ifndef GLOBALSCOPE_HPP
#define GLOBALSCOPE_HPP

#include "ast/type.hpp"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"

/**
 * Represents both a VSL and LLVM function.
 */
class FuncItem
{
public:
    /**
     * Creates a null FuncItem.
     */
    FuncItem();
    /**
     * Creates a FuncItem.
     *
     * @param type VSL type.
     * @param func LLVM function.
     */
    FuncItem(const FunctionType* type, llvm::Function* func);
    /**
     * Gets the VSL type.
     *
     * @returns VSL type.
     */
    const FunctionType* getVSLType() const;
    /**
     * Gets the LLVM function.
     *
     * @returns LLVM function.
     */
    llvm::Function* getLLVMFunc() const;
    /**
     * Checks if valid.
     *
     * @returns True if valid, false otherwise.
     */
    bool isValid() const;
    /**
     * Bool conversion.
     *
     * @returns True if valid, false otherwise.
     */
    operator bool() const;

private:
    /** VSL type. */
    const FunctionType* type;
    /** LLVM function. */
    llvm::Function* func;
};

/**
 * Manages stuff in the the global scope, like functions and what not.
 */
class GlobalScope
{
public:
    /**
     * Gets the function associated with a name. If it can't be found, a null
     * FuncItem is constructed and returned.
     *
     * @param name Name of the function.
     *
     * @returns The FuncItem associated with the given function name.
     */
    FuncItem getFunc(llvm::StringRef name) const;
    /**
     * Sets a name to be associated with a function.
     *
     * @param name Name of the function.
     * @param type VSL type of the function.
     * @param func LLVM function.
     *
     * @returns False if the operation succeeded, true otherwise.
     */
    bool setFunc(llvm::StringRef name, const FunctionType* type,
        llvm::Function* func);

private:
    /** Symbol table of all the functions. */
    llvm::StringMap<FuncItem> funcs;
};

#endif // GLOBALSCOPE_HPP
