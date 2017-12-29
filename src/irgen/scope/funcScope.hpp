#ifndef FUNCSCOPE_HPP
#define FUNCSCOPE_HPP

#include "ast/type.hpp"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Value.h"
#include <vector>

/**
 * Represents a variable (or parameter) within a scope.
 */
class VarItem
{
public:
    /**
     * Creates a null VarItem.
     */
    VarItem();
    /**
     * Creates a VarItem.
     *
     * @param type VSL type.
     * @param value LLVM value.
     */
    VarItem(const Type* type, llvm::Value* value);
    /**
     * Gets the VSL type.
     *
     * @returns VSL type.
     */
    const Type* getVSLType() const;
    /**
     * Gets the LLVM value.
     *
     * @returns LLVM value.
     */
    llvm::Value* getLLVMValue() const;
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
    const Type* type;
    /** LLVM value. */
    llvm::Value* value;
};

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
     * @returns The VarItem associated with the given variable name.
     */
    VarItem get(llvm::StringRef name) const;
    /**
     * Sets a name to be associated with a variable.
     *
     * @param name Name of the variable.
     * @param type VSL type of the variable.
     * @param value LLVM value.
     *
     * @returns False if the operation succeeded, true otherwise.
     */
    bool set(llvm::StringRef name, const Type* type, llvm::Value* value);
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
    /** Symbol table of variables. */
    using symtab_t = llvm::StringMap<VarItem>;
    /** List of symbol tables in a scope, managed like a stack. */
    std::vector<symtab_t> vars;
    /** The type that this function is supposed to return. */
    const Type* returnType;
};

#endif // FUNCSCOPE_HPP
