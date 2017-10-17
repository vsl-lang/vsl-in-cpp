#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "ast/type.hpp"
#include "llvm/IR/Value.h"
#include <string>
#include <unordered_map>

/**
 * Encapsulates a single level of scope. Basically a symbol table.
 */
class Scope
{
public:
    /**
     * Contains data associated with an identifier in the symbol table.
     */
    struct Item
    {
        /**
         * Tests equality.
         *
         * @param rhs The object to test equality with.
         *
         * @returns True if equal, otherwise false.
         */
        bool operator==(const Item& rhs) const;
        /**
         * Tests inequality.
         *
         * @param rhs The object to test inequality with.
         *
         * @returns True if not equal, otherwise false.
         */
        bool operator!=(const Item& rhs) const;
        /** VSL Type. */
        Type* type;
        /** LLVM Value. */
        llvm::Value* value;
    };
    /**
     * Creates a Scope.
     *
     * @param returnType The scope's return type. Used when verifying a return
     * statement.
     */
    Scope(Type* returnType);
    /**
     * Gets an Item from the symbol table.
     *
     * @param s The name of the Item.
     *
     * @returns The name's associated Item.
     */
    Item get(const std::string& s) const;
    /**
     * Sets `s` to be associated with `i`.
     *
     * @param s The name of the Item.
     * @param i The item to assign to `s`.
     *
     * @returns False if the symbol table already contains `s`, true otherwise.
     */
    bool set(const std::string& s, Item i);
    /**
     * Gets the scope's return type.
     *
     * @returns The scope's return type.
     */
    Type* getReturnType();

private:
    /** Internal symbol table. */
    std::unordered_map<std::string, Item> symtab;
    /** Scope's return type. */
    Type* returnType;
};

#endif // SCOPE_HPP
