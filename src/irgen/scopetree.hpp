#ifndef SCOPETREE_HPP
#define SCOPETREE_HPP

#include "ast/type.hpp"
#include "irgen/scope.hpp"
#include "llvm/IR/Value.h"
#include <string>
#include <vector>

/**
 * Encapsulates a 'tree' (stack) of {@link Scope Scopes}.
 */
class ScopeTree
{
public:
    /**
     * Creates a ScopeTree. Starts out with a single empty scope, the global
     * scope.
     */
    ScopeTree();
    /**
     * Gets an Item from the innermost scope that contains it.
     *
     * @param s The name of the Item.
     */
    Scope::Item get(const std::string& s) const;
    /**
     * Sets `s` to be associated with `i` in the innermost scope.
     *
     * @param s The name of the Item.
     * @param i The item to assign to `s`.
     *
     * @returns False if the scope already contains `s`, true otherwise.
     */
    bool set(const std::string& s, Scope::Item i);
    /**
     * Queries whether the current scope is the global scope.
     *
     * @returns True if the current scope is the global scope, false otherwise.
     */
    bool isGlobal() const;
    /**
     * Enters a new scope. The return type is the same as the last scope's
     * return type.
     */
    void enter();
    /**
     * Enters a new scope with a different return type.
     *
     * @param returnType The return type for the new scope.
     */
    void enter(const Type* returnType);
    /**
     * Exits the current scope.
     */
    void exit();
    /**
     * Gets the return type of the innermost scope.
     *
     * @returns The return type of the innermost scope.
     */
    const Type* getReturnType();

private:
    /** The list of scopes being encapsulated. */
    std::vector<Scope> scopes;
};

#endif // SCOPETREE_HPP
