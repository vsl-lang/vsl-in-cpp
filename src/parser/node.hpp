#ifndef NODE_HPP
#define NODE_HPP

class Node;
class ErrorNode;
class EmptyNode;
class BlockNode;
class ConditionalNode;
class AssignmentNode;
class FunctionNode;
class ReturnNode;
class ExprNode;
class IdentExprNode;
class NumberExprNode;
class UnaryExprNode;
class BinaryExprNode;
class CallExprNode;
class ArgNode;

#include "codegen/nodevisitor.hpp"
#include "lexer/location.hpp"
#include "lexer/token.hpp"
#include "parser/type.hpp"
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
        /** An error occured. */
        ERROR,
        /** Empty statement. */
        EMPTY,
        /** Code block. */
        BLOCK,
        /** If/else statement. */
        CONDITIONAL,
        /** Variable declaration. */
        ASSIGNMENT,
        /** Function. */
        FUNCTION,
        /** Return statement. */
        RETURN,
        /** Identifier. */
        ID_EXPR,
        /** Number (for now, just an integer). */
        NUMBER_EXPR,
        /** Unary expression. */
        UNARY_EXPR,
        /** Binary expression. */
        BINARY_EXPR,
        /** Function call. */
        CALL_EXPR,
        /** Function argument. */
        ARG
    };
    /**
     * Creates a Node object.
     *
     * @param kind The kind of Node this is.
     * @param location The source location.
     * @param type The VSL type this Node represents.
     */
    Node(Kind kind, Location location, std::unique_ptr<Type> type = nullptr);
    /**
     * Destroys a Node object.
     */
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
    /**
     * VSL Type that this Node contains. This is mostly for expressions and is
     * usually filled out by a {@link NodeVisitor} of some sort.
     */
    std::unique_ptr<Type> type;
    /** The kind of Node this is. */
    Kind kind;
    /** Where this Node was found in the source. */
    Location location;
};

/**
 * Represents a parser error.
 */
class ErrorNode : public Node
{
public:
    /**
     * Creates an ErrorNode.
     *
     * @param location Where this ErrorNode was found in the source.
     */
    ErrorNode(Location location);
    /**
     * Destroys an ErrorNode.
     */
    virtual ~ErrorNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
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
    /**
     * Destroys an ErrorNode.
     */
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
    BlockNode(std::vector<std::unique_ptr<Node>> statements, Location location);
    /**
     * Destroys a BlockNode.
     */
    virtual ~BlockNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The statements inside the block. */
    std::vector<std::unique_ptr<Node>> statements;
};

/**
 * Represents an if/else statement.
 */
class ConditionalNode : public Node
{
public:
    /**
     * Creates a ConditionalNode.
     *
     * @param condition The condition to test.
     * @param thenCase The code to run if the condition is true.
     * @param elseCase The code to run if the condition is false.
     * @param location Where this ConditionalNode was found in the source.
     */
    ConditionalNode(std::unique_ptr<Node> condition,
        std::unique_ptr<Node> thenCase, std::unique_ptr<Node> elseCase,
        Location location);
    /**
     * Destroys a ConditionalNode.
     */
    virtual ~ConditionalNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The condition to test. */
    std::unique_ptr<Node> condition;
    /** The code to run if the condition is true. */
    std::unique_ptr<Node> thenCase;
    /** The code to run if the condition is false. */
    std::unique_ptr<Node> elseCase;
};

/**
 * Represents a variable declaration.
 */
class AssignmentNode : public Node
{
public:
    /**
     * Bitflag enum for its type qualifiers. This should really be moved to Type
     * later, and it's currently not implemented yet.
     */
    enum Qualifiers
    {
        /** Nonconst. */
        NONCONST = 0,
        /** Constant. */
        CONST = 1
    };
    /**
     * Creates an AssignmentNode.
     *
     * @param name The name of the variable.
     * @param type The type of the variable.
     * @param value The variable's initial value.
     * @param qualifiers Type qualifiers for the variable.
     * @param location Where this AssignmentNode was found in the source.
     */
    AssignmentNode(std::string name, std::unique_ptr<Type> type,
        std::unique_ptr<Node> value, Qualifiers qualifiers, Location location);
    /**
     * Destroys an AssignmentNode.
     */
    virtual ~AssignmentNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The name of the variable. */
    std::string name;
    /** The variable's initial value. */
    std::unique_ptr<Node> value;
    /** Type qualifiers for the variable. */
    Qualifiers qualifiers;
};

/**
 * Represents a function.
 */
class FunctionNode : public Node
{
public:
    /**
     * Represents the name of a parameter. This is its own separate class
     * because {@link FunctionType} keeps track of the types, but
     * {@link FunctionNode::paramNames} keeps track of the parameter names and
     * locations.
     */
    struct ParamName
    {
        /**
         * Creates a ParamName.
         */
        ParamName() = default;
        /**
         * Creates a ParamName.
         *
         * @param str String representation of the name.
         * @param location where this ParamName was found in the source.
         */
        ParamName(std::string str, Location location);
        /** String representation of the name. */
        std::string str;
        /** Where this ParamName was found in the source. */
        Location location;
    };
    /**
     * Represents a function parameter.
     */
    struct Param
    {
        /** The name of the parameter. */
        ParamName name;
        /** The type of the parameter. */
        std::unique_ptr<Type> type;
    };
    /**
     * Creates a FunctionNode.
     *
     * @param name The name of the function.
     * @param params The function's parameters.
     * @param returnType The type that the function returns.
     * @param body The body of the function.
     * @param location Where this FunctionNode was found in the source.
     */
    FunctionNode(std::string name, std::vector<Param> params,
        std::unique_ptr<Type> returnType, std::unique_ptr<Node> body,
        Location location);
    /**
     * Destroys a FunctionNode.
     */
    virtual ~FunctionNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /**
     * Returns the string representation of a parameter.
     *
     * @param name The name of the parameter.
     * @param type The type of the parameter.
     *
     * @returns The string representation of a parameter.
     */
    static std::string paramToString(const std::string& name, const Type& type);
    /** The name of the function. */
    std::string name;
    /** The function's parameters. */
    std::vector<ParamName> paramNames;
    /** The body of the function. */
    std::unique_ptr<Node> body;
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
    ReturnNode(std::unique_ptr<Node> value, Location location);
    /**
     * Destroys a ReturnNode.
     */
    virtual ~ReturnNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The value to return. */
    std::unique_ptr<Node> value;
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
};

/**
 * Represents an identifier.
 */
class IdentExprNode : public ExprNode
{
public:
    /**
     * Creates an IdentExprNode.
     *
     * @param name The name of the identifier.
     * @param location where this IdentExprNode was found in the source.
     */
    IdentExprNode(std::string name, Location location);
    /**
     * Destroys an IdentExprNode.
     */
    virtual ~IdentExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The name of the identifier. */
    std::string name;
};

/**
 * Represents a number. For now, this is just a long int.
 */
class NumberExprNode : public ExprNode
{
public:
    /**
     * Creates a NumberExprNode.
     *
     * @param value The value of the number.
     * @param location Where this NumberExprNode was found in the source.
     */
    NumberExprNode(long value, Location location);
    /**
     * Destroys a NumberExprNode.
     */
    virtual ~NumberExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The value of the number. */
    long value;
};

/**
 * Represents a unary expression.
 */
class UnaryExprNode : public ExprNode
{
public:
    /**
     * Creates a UnaryExprNode.
     *
     * @param op The operator of the expression.
     * @param expr The expression to apply the operator to.
     * @param location Where this UnaryExprNode was found in the source.
     */
    UnaryExprNode(Token::Kind op, std::unique_ptr<Node> expr,
        Location location);
    /**
     * Destroys a UnaryExprNode.
     */
    virtual ~UnaryExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The operator of the expression. */
    Token::Kind op;
    /** The expression to apply the operator to. */
    std::unique_ptr<Node> expr;
};

/**
 * Represents a binary expression.
 */
class BinaryExprNode : public ExprNode
{
public:
    /**
     * Creates a BinaryExprNode.
     *
     * @param op The operator of the expression.
     * @param left The left hand side of the expression.
     * @param right The right hand side of the expression.
     * @param location Where this BinaryExprNode was found in the source.
     */
    BinaryExprNode(Token::Kind op, std::unique_ptr<Node> left,
        std::unique_ptr<Node> right, Location location);
    /**
     * Destroys a BinaryExprNode.
     */
    virtual ~BinaryExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The operator of the expression. */
    Token::Kind op;
    /** The left hand side of the expression. */
    std::unique_ptr<Node> left;
    /** The right hand side of the expression. */
    std::unique_ptr<Node> right;
};

/**
 * Represents a function call.
 */
class CallExprNode : public ExprNode
{
public:
    /**
     * Creates a CallExprNode.
     *
     * @param callee The function to call.
     * @param args The arguments to pass to the callee.
     * @param location Where this CallExprNode was found in the source.
     */
    CallExprNode(std::unique_ptr<Node> callee,
        std::vector<std::unique_ptr<Node>> args, Location location);
    /**
     * Destroys a CallExprNode.
     */
    virtual ~CallExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The function to call. */
    std::unique_ptr<Node> callee;
    /** The arguments to pass to the callee. */
    std::vector<std::unique_ptr<Node>> args;
};

/**
 * Represents a function argument.
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
    ArgNode(std::string name, std::unique_ptr<Node> value, Location location);
    /**
     * Destroys an ArgNode.
     */
    ~ArgNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    /** The name of the argument. */
    std::string name;
    /** The value of the argument. */
    std::unique_ptr<Node> value;
};

#endif // NODE_HPP
