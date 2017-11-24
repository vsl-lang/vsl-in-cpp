#ifndef NODE_HPP
#define NODE_HPP

class Node;
class EmptyNode;
class BlockNode;
class IfNode;
class VariableNode;
class FunctionNode;
class ParamNode;
class ReturnNode;
class ExprNode;
class VoidNode;
class IdentNode;
class LiteralNode;
class UnaryNode;
class BinaryNode;
class CallNode;
class ArgNode;

#include "ast/nodevisitor.hpp"
#include "ast/type.hpp"
#include "lexer/location.hpp"
#include "lexer/token.hpp"
#include "lexer/tokenKind.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

/**
 * Allows a {@link Node} to be printed to an output stream.
 *
 * @param os The stream to print to.
 * @param ast The Node to print.
 *
 * @returns The same output stream that was given.
 */
std::ostream& operator<<(std::ostream& os, const Node& ast);

/**
 * Base class for the VSL AST.
 */
class Node
{
public:
    /**
     * Identifies the kind of Node a given object is. Keep in mind that one
     * subclass can represent multiple Kinds of Nodes.
     */
    enum Kind
    {
        /** Empty statement. */
        EMPTY,
        /** Code block. */
        BLOCK,
        /** If/else statement. */
        IF,
        /** Variable definition. */
        VARIABLE,
        /** Function. */
        FUNCTION,
        /** Return statement. */
        RETURN,
        /** Identifier. */
        IDENT,
        /** Integer literal. */
        LITERAL,
        /** Void (nullary) expression. */
        VOID,
        /** Unary expression. */
        UNARY,
        /** Binary expression. */
        BINARY,
        /** Function call. */
        CALL,
        /** Function parameter. */
        PARAM,
        /** Function call argument. */
        ARG,
    };
    /**
     * Creates a Node object.
     *
     * @param kind The kind of Node this is.
     * @param location The source location.
     */
    Node(Kind kind, Location location);
    virtual ~Node() = 0;
    /**
     * Allows a {@link NodeVisitor} to visit this object.
     *
     * @param nodeVisitor The visitor object.
     */
    virtual void accept(NodeVisitor& nodeVisitor) = 0;
    /**
     * Returns the string representation of this Node.
     *
     * @returns The string representation of this Node.
     */
    virtual std::string toString() const = 0;
    /** The kind of Node this is. */
    Kind kind;
    /** Where this Node was found in the source. */
    Location location;
};

/**
 * Represents an empty statement.
 */
class EmptyNode : public Node
{
public:
    /**
     * Creates an EmptyNode.
     *
     * @param location Where this EmptyNode was found in the source.
     */
    EmptyNode(Location location);
    virtual ~EmptyNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
};

/**
 * Represents a block of code.
 */
class BlockNode : public Node
{
public:
    /**
     * Creates a BlockNode.
     *
     * @param statements The statements inside the block.
     * @param location Where this BlockNode was found in the source.
     */
    BlockNode(std::vector<Node*> statements, Location location);
    virtual ~BlockNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The statements inside the block. */
    std::vector<Node*> statements;
};

/**
 * Represents an if/else statement.
 */
class IfNode : public Node
{
public:
    /**
     * Creates a IfNode.
     *
     * @param condition The condition to test.
     * @param thenCase The code to run if the condition is true.
     * @param elseCase The code to run if the condition is false.
     * @param location Where this IfNode was found in the source.
     */
    IfNode(ExprNode* condition, Node* thenCase, Node* elseCase,
        Location location);
    virtual ~IfNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The condition to test. */
    ExprNode* condition;
    /** The code to run if the condition is true. */
    Node* thenCase;
    /** The code to run if the condition is false. */
    Node* elseCase;
};

/**
 * Represents a variable declaration.
 */
class VariableNode : public Node
{
public:
    /**
     * Creates a VariableNode.
     *
     * @param name The name of the variable.
     * @param type The type of the variable.
     * @param value The variable's initial value.
     * @param isConst If this variable is const or not.
     * @param location Where this VariableNode was found in the source.
     */
    VariableNode(llvm::StringRef name, const Type* type, ExprNode* value,
        bool isConst, Location location);
    virtual ~VariableNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The name of the variable. */
    llvm::StringRef name;
    /** The type of the variable. */
    const Type* type;
    /** The variable's initial value. */
    ExprNode* value;
    /** If this variable is const or not. */
    bool isConst;
};

/**
 * Represents a function.
 */
class FunctionNode : public Node
{
public:
    /**
     * Creates a FunctionNode.
     *
     * @param name The name of the function.
     * @param params The function's parameters.
     * @param returnType The type that the function returns.
     * @param body The body of the function.
     * @param location Where this FunctionNode was found in the source.
     */
    FunctionNode(llvm::StringRef name, std::vector<ParamNode*> params,
        const Type* returnType, Node* body, Location location);
    virtual ~FunctionNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The name of the function. */
    llvm::StringRef name;
    /** The function's parameters. */
    std::vector<ParamNode*> params;
    /** The function's return type. */
    const Type* returnType;
    /** The body of the function. */
    Node* body;
};

/**
 * Represents a function parameter.
 */
class ParamNode : public Node
{
public:
    /**
     * Creates a ParamNode.
     *
     * @param name The name of the parameter.
     * @param type The type of the parameter.
     * @param location Where this ParamNode was found in the source.
     */
    ParamNode(llvm::StringRef name, const Type* type, Location location);
    virtual ~ParamNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The name of the parameter. */
    llvm::StringRef name;
    /** The type of the parameter. */
    const Type* type;
};

/**
 * Represents a return statement.
 */
class ReturnNode : public Node
{
public:
    /**
     * Creates a ReturnNode.
     *
     * @param value The value to return.
     * @param location Where this ReturnNode was found in the source.
     */
    ReturnNode(ExprNode* value, Location location);
    virtual ~ReturnNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The value to return. */
    ExprNode* value;
};

/**
 * Base class for expressions. This may not actually be needed if no one takes
 * advantage of the inheritance relationship.
 */
class ExprNode : public Node
{
public:
    /**
     * Creates an ExprNode.
     *
     * @param kind The kind of ExprNode this is.
     * @param location where this ExprNode was found in the source.
     */
    ExprNode(Node::Kind kind, Location location);
    /**
     * VSL Type that this ExprNode contains. This is usually filled in later by
     * a {@link NodeVisitor} of some sort.
     */
    const Type* type;
};

/**
 * Represents an identifier.
 */
class IdentNode : public ExprNode
{
public:
    /**
     * Creates an IdentNode.
     *
     * @param name The name of the identifier.
     * @param location where this IdentNode was found in the source.
     */
    IdentNode(llvm::StringRef name, Location location);
    virtual ~IdentNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The name of the identifier. */
    llvm::StringRef name;
};

/**
 * Represents an (integer) literal.
 */
class LiteralNode : public ExprNode
{
public:
    /**
     * Creates an LiteralNode.
     *
     * @param value The value of the number.
     * @param location Where this LiteralNode was found in the source.
     */
    LiteralNode(llvm::APInt value, Location location);
    virtual ~LiteralNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The value of the number. */
    llvm::APInt value;
};

/**
 * Represents a void (nullary) expression.
 */
class VoidNode : public ExprNode
{
public:
    /**
     * Creates a VoidNode.
     *
     * @param location Where this VoidNode was found in the source.
     */
    VoidNode(Location location);
    virtual ~VoidNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
};

/**
 * Represents a unary expression.
 */
class UnaryNode : public ExprNode
{
public:
    /**
     * Creates a UnaryNode.
     *
     * @param op The operator of the expression.
     * @param expr The expression to apply the operator to.
     * @param location Where this UnaryNode was found in the source.
     */
    UnaryNode(TokenKind op, ExprNode* expr, Location location);
    virtual ~UnaryNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The operator of the expression. */
    TokenKind op;
    /** The expression to apply the operator to. */
    ExprNode* expr;
};

/**
 * Represents a binary expression.
 */
class BinaryNode : public ExprNode
{
public:
    /**
     * Creates a BinaryNode.
     *
     * @param op The operator of the expression.
     * @param left The left hand side of the expression.
     * @param right The right hand side of the expression.
     * @param location Where this BinaryNode was found in the source.
     */
    BinaryNode(TokenKind op, ExprNode* left, ExprNode* right,
        Location location);
    virtual ~BinaryNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The operator of the expression. */
    TokenKind op;
    /** The left hand side of the expression. */
    ExprNode* left;
    /** The right hand side of the expression. */
    ExprNode* right;
};

/**
 * Represents a function call.
 */
class CallNode : public ExprNode
{
public:
    /**
     * Creates a CallNode.
     *
     * @param callee The function to call.
     * @param args The arguments to pass to the callee.
     * @param location Where this CallNode was found in the source.
     */
    CallNode(ExprNode* callee, std::vector<ArgNode*> args, Location location);
    virtual ~CallNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The function to call. */
    ExprNode* callee;
    /** The arguments to pass to the callee. */
    std::vector<ArgNode*> args;
};

/**
 * Represents a function call argument.
 */
class ArgNode : public Node
{
public:
    /**
     * Creates an ArgNode.
     *
     * @param name The name of the argument. Used for name parameters (WIP).
     * @param value The value of the argument.
     * @param location Where this ArgNode was found in the source.
     */
    ArgNode(llvm::StringRef name, ExprNode* value, Location location);
    ~ArgNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The name of the argument. */
    llvm::StringRef name;
    /** The value of the argument. */
    ExprNode* value;
};

#endif // NODE_HPP
