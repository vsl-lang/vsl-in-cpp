#ifndef FUNCSCOPE_HPP
#define FUNCSCOPE_HPP

#include "ast/type.hpp"
#include "irgen/value/value.hpp"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include <utility>
#include <vector>

/**
 * Manages the multiple scopes in a function body.
 */
class FuncScope
{
public:
    /**
     * Represents a variable. Contains its name and Value (guaranteed to be a
     * variable Value).
     */
    using VarItem = std::pair<llvm::StringRef, Value>;

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
     * Gets a list of all variables in the current scope.  The beginning of the
     * ArrayRef contains the oldest variable, and the end contains the newest.
     *
     * The returned ArrayRef is invalidated if a new variable is defined
     * afterwards.
     */
    llvm::ArrayRef<VarItem> getVars() const;
    /**
     * Gets a list of all variables in the entire FuncScope, separated by scope
     * level. Newer scopes are towards the end of the vector, while older ones
     * are in the front. In each ArrayRef, newer variables are towards the end
     * etc.
     *
     * After returning, exiting a scope or modifying it will invalidate the
     * ArrayRef that represents that scope.
     */
    std::vector<llvm::ArrayRef<VarItem>> getAllVars() const;
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
    /**
     * Represents a symbol table. This is a MapVector so that insertion order is
     * kept when deleting variables when exiting the scope.
     */
    using SymbolTable = llvm::MapVector<llvm::StringRef, Value,
        llvm::StringMap<unsigned>, std::vector<VarItem>>;

    /** List of symbol tables in a scope, managed like a stack. */
    std::vector<SymbolTable> vars;
    /** The type that this function is supposed to return. */
    const Type* returnType;
};

#endif // FUNCSCOPE_HPP
