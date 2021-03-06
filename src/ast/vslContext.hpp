#ifndef VSLCONTEXT_HPP
#define VSLCONTEXT_HPP

#include "ast/node.hpp"
#include "ast/type.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringMap.h"
#include <deque>
#include <unordered_set>
#include <vector>

/**
 * Context object that owns and manages the AST and other objects related to it.
 * All {@link Node Nodes} and {@link Type Types} are unique so it's fine to
 * compare pointers to them for equality rather than the actual objects.
 */
class VSLContext
{
public:
    VSLContext();

    /**
     * @name Node Getters
     * @{
     */

    /**
     * Transfers ownership of a Node to this object.
     */
    void addNode(std::unique_ptr<Node> node);
    /**
     * Indicate that a DeclNode is in the global scope. It's not recommended to
     * call this multiple times with the same pointer.
     */
    void setGlobal(DeclNode* decl);
    /**
     * Gets a reference to the current list of global DeclNodes. If setGlobal is
     * called afterwards, the returned ArrayRef may or may not be valid.
     */
    llvm::ArrayRef<DeclNode*> getGlobals() const;

    /**
     * @}
     * @name Type Getters
     * @{
     */

    const SimpleType* getBoolType() const;
    const SimpleType* getIntType() const;
    const SimpleType* getVoidType() const;
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
     * Checks if a named type already exists.
     *
     * @param naem Name of the type.
     *
     * @returns True if the type does exist, false otherwise.
     */
    bool hasNamedType(llvm::StringRef name) const;
    /**
     * Gets a named type or creates one it nonexistent.
     *
     * @param name Name of the type.
     *
     * @returns A named type.
     */
    const NamedType* getNamedType(llvm::StringRef name);
    /**
     * Gets or constructs a FunctionType from a FuncInterfaceNode. This is a
     * relatively expensive operation so it should only be called once per
     * function max. Note that multiple FuncInterfaceNodes can have the same
     * FunctionType.
     *
     * @param node Function to get the type of.
     *
     * @returns A FunctionType from the given function.
     */
    const FunctionType* getFunctionType(const FuncInterfaceNode& node);
    /**
     * Creates a named type. Returns null if the name already exists.
     *
     * @param name Name of the type.
     *
     * @returns A new NamedType, or null if the name already exists.
     */
    const NamedType* createNamedType(llvm::StringRef name);
    /**
     * Constructs an opaque, mutable class type. This can be used along with its
     * mutator methods to build the "body" of the class type.
     *
     * @returns A new class type.
     */
    ClassType* createClassType();

    /** @} */

private:
    /**
     * Gets the parameter types of a function.
     *
     * @param node Function to get the parameter types of.
     *
     * @returns Parameter types wrapped in a vector.
     */
    static std::vector<const Type*> getParamTypes(
        const FuncInterfaceNode& node);
    /** Owns all the Nodes. */
    std::deque<std::unique_ptr<Node>> nodes;
    /** Contains all global declarations in order. */
    std::vector<DeclNode*> globals;
    /** Placeholder for any type errors. */
    SimpleType errorType;
    /** Represents the Bool type. */
    SimpleType boolType;
    /** Represents the Int type. */
    SimpleType intType;
    /** Represents the Void type. */
    SimpleType voidType;
    /** Contains all the Namedtypes. */
    std::unordered_set<NamedType> namedTypes;
    /** Contains all the FunctionTypes. */
    std::unordered_set<FunctionType> functionTypes;
    /** Contains all the ClassTypes. */
    std::deque<ClassType> classTypes;
};

#endif // VSLCONTEXT_HPP
