#ifndef GLOBALSCOPE_HPP
#define GLOBALSCOPE_HPP

#include "ast/node.hpp"
#include "ast/type.hpp"
#include "irgen/value/value.hpp"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include <unordered_map>
#include <utility>

/**
 * Manages objects in the the global scope, like functions and what not.
 */
class GlobalScope
{
public:
    /**
     * @name Getters
     * @{
     */

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
     * Gets the ctor associated with a type. If it can't be found, a pair
     * containing a null Value and `Access::NONE` is constructed and returned.
     *
     * @param type The type that has the method.
     *
     * @returns The constructor Value associated with the given type along with
     * its access specifier.
     */
    std::pair<Value, Access> getCtor(const Type* type) const;
    /**
     * Gets the method associated with a type and name. If it can't be found, a
     * pair containing a null Value and `Access::NONE` is constructed and
     * returned.
     *
     * @param type The type that has the method.
     * @param method Name of the method.
     *
     * @returns The method Value associated with the given object name along
     * with its access specifier.
     */
    std::pair<Value, Access> getMethod(const Type* type,
        llvm::StringRef method) const;
    /**
     * Gets the destructor associated with a type. If it can't be found, null is
     * returned.
     *
     * @param type Type to destruct.
     *
     * @returns The LLVM Function that destructs the given type, or null if
     * nonexistent.
     */
    llvm::Function* getDtor(const Type* type) const;

    /**
     * @}
     * @name Setters
     * @{
     */

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
    /**
     * Sets the constructor for the given type. This method returns true if the
     * constructor is already set.
     *
     * @param type Type to set the constructor of.
     * @param vslFunc VSL function type. Must be a constructor type.
     * @param llvmFunc LLVM function with implicit self parameter.
     * @param access Access specifier.
     *
     * @returns False if successful, true if the constructor already exists.
     */
    bool setCtor(const Type* type, const FunctionType* vslFunc,
        llvm::Function* llvmFunc, Access access);
    /**
     * Adds a method to a given type. This method fails and returns true if the
     * method name already exists.
     *
     * @param type Type to add the method to.
     * @param name Method name.
     * @param vslFunc VSL function type. Must be a method type.
     * @param llvmFunc LLVM function with implicit self parameter.
     * @param access Access specifier.
     *
     * @returns False if successful, true if the method already exists.
     */
    bool setMethod(const Type* type, llvm::StringRef name,
        const FunctionType* vslFunc, llvm::Function* llvmFunc, Access access);
    /**
     * Sets the destructor function of a given type. The function should take
     * only a pointer to the type and return void. This method returns true if
     * the destructor already exists.
     *
     * @param type Type to set the destructor of.
     * @param llvmFunc LLVM function.
     *
     * @returns False if successful, true if the destructor already exists.
     */
    bool setDtor(const Type* type, llvm::Function* llvmFunc);

    /** @} */

private:
    /** Symbol table of all the global objects. */
    llvm::StringMap<Value> symtab;
    /** Constructors defined for every type. */
    std::unordered_map<const Type*, std::pair<Value, Access>> ctors;
    /** Methods defined for each type along with access specifier. */
    std::unordered_map<const Type*,
        llvm::StringMap<std::pair<Value, Access>>> methods;
    /** Destructor defined for every type. */
    std::unordered_map<const Type*, llvm::Function*> dtors;
};

#endif // GLOBALSCOPE_HPP
