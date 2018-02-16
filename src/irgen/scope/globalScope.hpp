#ifndef GLOBALSCOPE_HPP
#define GLOBALSCOPE_HPP

#include "ast/type.hpp"
#include "irgen/value/value.hpp"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"

/**
 * Manages objects in the the global scope, like functions and what not.
 */
class GlobalScope
{
public:
    /**
     * Gets the object associated with a name. If it can't be found, a null
     * Value is constructed and returned.
     *
     * @param name Name of the function.
     *
     * @returns The Value associated with the given object name.
     */
    Value get(llvm::StringRef name) const;
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
    /**
     * Sets a name to be associated with a variable.
     *
     * @param name Name of the variable.
     * @param type VSL type of the variable.
     * @param func LLVM variable.
     *
     * @returns False if the operation succeeded, true otherwise.
     */
    bool setVar(llvm::StringRef name, const Type* type,
        llvm::Value* var);

private:
    /** Symbol table of all the global objects. */
    llvm::StringMap<Value> symtab;
};

#endif // GLOBALSCOPE_HPP
