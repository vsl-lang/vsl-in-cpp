#ifndef VSLCONTEXT_HPP
#define VSLCONTEXT_HPP

#include "ast/node.hpp"
#include "ast/type.hpp"
#include "lexer/location.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/raw_ostream.h"
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
     * Gets or constructs a FunctionType. This is a relatively expensive
     * operation so it should only be called once per function max.
     *
     * @param params The list of parameters.
     * @param returnType The return type.
     *
     * @returns A FunctionType using the given parameters.
     */
    const FunctionType* getFunctionType(std::vector<const Type*> params,
        const Type* returnType);
    /**
     * Gets or constructs a FunctionType from a FuncInterfaceNode.
     *
     * @param node Function to get the type of.
     *
     * @returns A FunctionType from the given function.
     */
    const FunctionType* getFunctionType(FuncInterfaceNode& node);

    /** @} */

private:
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
    /** Contains all the FunctionTypes. */
    std::unordered_set<FunctionType> functionTypes;
};

#endif // VSLCONTEXT_HPP
