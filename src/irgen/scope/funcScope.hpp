#ifndef FUNCSCOPE_HPP
#define FUNCSCOPE_HPP

#include "ast/type.hpp"
#include "irgen/value/value.hpp"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Value.h"
#include <vector>

/**
 * Manages the multiple scopes in a function body.
 */
class FuncScope
{
public:
    /**
     * Creates a FuncScope.
     */
    FuncScope();
    /**
     * Gets the variable associated with a name. If it can't be found, a null
     * VarItem is constructed and returned.
     *
     * @param name Name of the variable.
     *
     * @returns The Value associated with the given variable name.
     */
    Value get(llvm::StringRef name) const;
    /**
     * Sets a name to be associated with a variable.
     *
     * @param name Name of the variable.
     * @param value Value of the variable.
     *
     * @returns False if the operation succeeded, true otherwise.
     */
    bool set(llvm::StringRef name, Value value);
    /**
     * Enters a new scope. The return type is the same as the last scope's
     * return type.
     */
    void enter();
    /**
     * Exits the current scope.
     */
    void exit();
    /**
     * Checks if there are no scopes on the stack.
     *
     * @returns True if no scopes on the stack, false otherwise.
     */
    bool empty() const;
    /**
     * Gets the return type.
     *
     * @returns Return type.
     */
    const Type* getReturnType() const;
    /**
     * Sets the return type.
     *
     * @param returnType New return type.
     */
    void setReturnType(const Type* returnType);

private:
    /** List of symbol tables in a scope, managed like a stack. */
    std::vector<llvm::StringMap<Value>> vars;
    /** The type that this function is supposed to return. */
    const Type* returnType;
};

#endif // FUNCSCOPE_HPP
