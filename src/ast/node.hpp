#ifndef NODE_HPP
#define NODE_HPP

class Node;
class DeclNode;
class FuncInterfaceNode;
class FunctionNode;
class ExtFuncNode;
class ParamNode;
class VariableNode;
class BlockNode;
class EmptyNode;
class IfNode;
class ReturnNode;
class ExprNode;
class IdentNode;
class LiteralNode;
class UnaryNode;
class BinaryNode;
class TernaryNode;
class CallNode;
class ArgNode;

#include "ast/nodevisitor.hpp"
#include "ast/opKind.hpp"
#include "ast/type.hpp"
#include "lexer/location.hpp"
#include "lexer/tokenKind.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include <string>
#include <vector>

/**
 * Types of access modifiers.
 */
enum class AccessMod
{
    /** Can be accessed anywhere. */
    PUBLIC,
    /** Can be accessed only in context it was declared. */
    PRIVATE,
    /** Not applicable, e.g. inside a function. */
    NONE
};

/**
 * Base class for the VSL AST.
 */
class Node
{
    friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os,
        const Node& node);
public:
    /**
     * Identifies the kind of Node a given object is. Keep in mind that one
     * subclass can represent multiple Kinds of Nodes.
     */
    enum Kind
    {
        /** Function. */
        FUNCTION,
        /** External function. */
        EXTFUNC,
        /** Code block. */
        BLOCK,
        /** Empty statement. */
        EMPTY,
        /** Variable definition. */
        VARIABLE,
        /** If/else statement. */
        IF,
        /** Return statement. */
        RETURN,
        /** Identifier. */
        IDENT,
        /** Integer literal. */
        LITERAL,
        /** Unary expression. */
        UNARY,
        /** Binary expression. */
        BINARY,
        /** Ternary expression. */
        TERNARY,
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
     * Allows a NodeVisitor to visit this object.
     *
     * @param nodeVisitor The visitor object.
     */
    virtual void accept(NodeVisitor& nodeVisitor) = 0;
    /**
     * Verifies whether this Node represents a certain Kind.
     *
     * @param k The Kind to check for.
     *
     * @returns True if this Node represents the given Kind, false otherwise.
     */
    bool is(Kind k) const;
    bool isNot(Kind k) const;
    /**
     * Gets the location info.
     *
     * @returns Where this Node was found in the source.
     */
    Location getLoc() const;
    /**
     * Checks whether this Node is an expression.
     *
     * @returns True if this Node is a subclass of ExprNode, false otherwise.
     */
    virtual bool isExpr() const;

private:
    /** The kind of Node this is. */
    Kind kind;
    /** Where this Node was found in the source. */
    Location location;
};

/**
 * Represents a declaration/definition with an access modifier.
 */
class DeclNode : public Node
{
public:
    /**
     * Creates a DeclNode.
     *
     * @param kind The kind of Node this is.
     * @param location Where this DeclNode was found in the source.
     * @param access Access modifier.
     */
    DeclNode(Node::Kind kind, Location location, AccessMod access);
    /**
     * Gets the access modifier.
     *
     * @returns The access modifier.
     */
    AccessMod getAccessMod() const;

private:
    /** Access modifier. */
    AccessMod access;
};

/**
 * Represents a function interface, e.g.\ `func f(x: Int) -> Int`. Basically a
 * function without a body, which can be either defined by a FunctionNode or
 * declared external by an ExtFuncNode.
 */
class FuncInterfaceNode : public DeclNode
{
public:
    /**
     * Creates a FuncInterfaceNode.
     *
     * @param kind Node kind.
     * @param location Where this FuncInterfaceNode was found in the source.
     * @param access Access modifier.
     * @param name The name of the function.
     * @param params The function's parameters.
     * @param returnType The type that the function returns.
     * @param ft Used for symbol table lookup.
     */
    FuncInterfaceNode(Node::Kind kind, Location location, AccessMod access,
        llvm::StringRef name, std::vector<ParamNode*> params,
        const Type* returnType, const FunctionType* ft);
    llvm::StringRef getName() const;
    llvm::ArrayRef<ParamNode*> getParams() const;
    size_t getNumParams() const;
    ParamNode* getParam(size_t i) const;
    const Type* getReturnType() const;
    const FunctionType* getFuncType() const;

protected:
    /**
     * Gets the string representation of the name, parameters, and return type.
     *
     * @param indent Indentation level.
     *
     * @returns String representation.
     */
    std::string toStr(int indent) const;

private:
    /** The name of the function. */
    llvm::StringRef name;
    /** The function's parameters. */
    std::vector<ParamNode*> params;
    /** The function's return type. */
    const Type* returnType;
    /** Used for symbol table lookup. */
    const FunctionType* ft;
};

/**
 * Represents a function definition, e.g.\ `func f(x: Int) -> Int { ... }`.
 */
class FunctionNode : public FuncInterfaceNode
{
public:
    /**
     * Creates a FunctionNode.
     *
     * @param location Where this FunctionNode was found in the source.
     * @param access Access modifier.
     * @param name The name of the function.
     * @param params The function's parameters.
     * @param returnType The type that the function returns.
     * @param ft Used for symbol table lookup.
     * @param body The body of the function.
     */
    FunctionNode(Location location, AccessMod access, llvm::StringRef name,
        std::vector<ParamNode*> params, const Type* returnType,
        const FunctionType* ft, Node* body);
    virtual ~FunctionNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    Node* getBody() const;
    bool isAlreadyDefined() const;
    void setAlreadyDefined(bool alreadyDefined = true);

private:
    Node* body;
    /** Whether this function was already defined. */
    bool alreadyDefined;
};

/**
 * Represents an external function, e.g.\ `func f(x: Int) -> Int external(g);`.
 * VSL programs will use the function name `f`, but outside of that it's
 * referred to by its alias `g`.
 */
class ExtFuncNode : public FuncInterfaceNode
{
public:
    /**
     * Creates an ExtFuncNode.
     *
     * @param location Where this FunctionNode was found in the source.
     * @param access Access modifier.
     * @param name The name of the function.
     * @param params The function's parameters.
     * @param returnType The type that the function returns.
     * @param ft Used for symbol table lookup.
     * @param alias What this function's actual name is outside of VSL.
     */
    ExtFuncNode(Location location, AccessMod access, llvm::StringRef name,
        std::vector<ParamNode*> params, const Type* returnType,
        const FunctionType* ft, llvm::StringRef alias);
    virtual ~ExtFuncNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getAlias() const;

private:
    /** The name this function is aliased to. */
    llvm::StringRef alias;
};

/**
 * Represents a function parameter, e.g.\ `x: Int`.
 */
class ParamNode : public Node
{
public:
    /**
     * Creates a ParamNode.
     *
     * @param location Where this ParamNode was found in the source.
     * @param name The name of the parameter.
     * @param type The type of the parameter.
     */
    ParamNode(Location location, llvm::StringRef name, const Type* type);
    virtual ~ParamNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getName() const;
    const Type* getType() const;

private:
    /** The name of the parameter. */
    llvm::StringRef name;
    /** The type of the parameter. */
    const Type* type;
};

/**
 * Represents a variable declaration, e.g.\ `var x: Int = 5;`.
 */
class VariableNode : public DeclNode
{
public:
    /**
     * Creates a VariableNode.
     *
     * @param location Where this VariableNode was found in the source.
     * @param access Access modifier.
     * @param name The name of the variable.
     * @param type The type of the variable.
     * @param init The variable's initial value.
     * @param constness If this variable is const or not (TODO).
     */
    VariableNode(Location location, AccessMod access, llvm::StringRef name,
        const Type* type, ExprNode* init, bool constness);
    virtual ~VariableNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getName() const;
    const Type* getType() const;
    ExprNode* getInit() const;
    bool isConst() const;

private:
    /** The name of the variable. */
    llvm::StringRef name;
    /** The type of the variable. */
    const Type* type;
    /** The variable's initial value. */
    ExprNode* init;
    /** If this variable is const or not. */
    bool constness;
};

/**
 * Represents a block of code, e.g.\ `{ ... }`.
 */
class BlockNode : public Node
{
public:
    /**
     * Creates a BlockNode.
     *
     * @param location Where this BlockNode was found in the source.
     * @param statements The statements inside the block.
     */
    BlockNode(Location location, std::vector<Node*> statements);
    virtual ~BlockNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::ArrayRef<Node*> getStatements() const;

private:
    /** The statements inside the block. */
    std::vector<Node*> statements;
};

/**
 * Represents an empty statement, e.g.\ `;`.
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
};

/**
 * Represents an if/else statement, e.g.\ `if (x) { ... } else { ... }`.
 */
class IfNode : public Node
{
public:
    /**
     * Creates a IfNode.
     *
     * @param location Where this IfNode was found in the source.
     * @param condition The condition to test.
     * @param thenCase The code to run if the condition is true.
     * @param elseCase The code to run if the condition is false.
     */
    IfNode(Location location, ExprNode* condition, Node* thenCase,
        Node* elseCase);
    virtual ~IfNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    ExprNode* getCondition() const;
    Node* getThen() const;
    Node* getElse() const;

private:
    /** The condition to test. */
    ExprNode* condition;
    /** The code to run if the condition is true. */
    Node* thenCase;
    /** The code to run if the condition is false. */
    Node* elseCase;
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
     * @param location Where this ReturnNode was found in the source.
     * @param value The value to return.
     */
    ReturnNode(Location location, ExprNode* value);
    virtual ~ReturnNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    bool hasValue() const;
    ExprNode* getValue() const;

private:
    /** The value to return. */
    ExprNode* value;
};

/**
 * Base class for expressions.
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
    virtual bool isExpr() const override;
};

/**
 * Represents an identifier, e.g.\ `x`.
 */
class IdentNode : public ExprNode
{
public:
    /**
     * Creates an IdentNode.
     *
     * @param location where this IdentNode was found in the source.
     * @param name The name of the identifier.
     */
    IdentNode(Location location, llvm::StringRef name);
    virtual ~IdentNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getName() const;

private:
    /** The name of the identifier. */
    llvm::StringRef name;
};

/**
 * Represents an (integer) literal, e.g.\ `1337`.
 */
class LiteralNode : public ExprNode
{
public:
    /**
     * Creates an LiteralNode.
     *
     * @param location Where this LiteralNode was found in the source.
     * @param value The value of the number.
     */
    LiteralNode(Location location, llvm::APInt value);
    virtual ~LiteralNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::APInt getValue() const;

private:
    /** The value of the number. */
    llvm::APInt value;
};

/**
 * Represents a unary expression, e.g.\ `-1`.
 */
class UnaryNode : public ExprNode
{
public:
    /**
     * Creates a UnaryNode.
     *
     * @param location Where this UnaryNode was found in the source.
     * @param op The operator of the expression.
     * @param expr The expression to apply the operator to.
     */
    UnaryNode(Location location, UnaryKind op, ExprNode* expr);
    virtual ~UnaryNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    UnaryKind getOp() const;
    const char* getOpSymbol() const;
    ExprNode* getExpr() const;

private:
    /** The operator of the expression. */
    UnaryKind op;
    /** The expression to apply the operator to. */
    ExprNode* expr;
};

/**
 * Represents a binary expression, e.g.\ `1+1`.
 */
class BinaryNode : public ExprNode
{
public:
    /**
     * Creates a BinaryNode.
     *
     * @param location Where this BinaryNode was found in the source.
     * @param op The operator of the expression.
     * @param left The left hand side of the expression.
     * @param right The right hand side of the expression.
     */
    BinaryNode(Location location, BinaryKind op, ExprNode* left,
        ExprNode* right);
    virtual ~BinaryNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    BinaryKind getOp() const;
    const char* getOpSymbol() const;
    ExprNode* getLhs() const;
    ExprNode* getRhs() const;

private:
    /** The operator of the expression. */
    BinaryKind op;
    /** The left hand side of the expression. */
    ExprNode* left;
    /** The right hand side of the expression. */
    ExprNode* right;
};

/**
 * Represents a ternary expression, e.g.\ `c ? x : y`.
 */
class TernaryNode : public ExprNode
{
public:
    /**
     * Creates a TernaryNode.
     *
     * @param location Where this TernaryNode was found in the source.
     * @param condition The condition to test for.
     * @param thenCase The expression when the condition is true.
     * @param elseCase The expression when the condition is false.
     */
    TernaryNode(Location location, ExprNode* condition, ExprNode* thenCase,
        ExprNode* elseCase);
    virtual ~TernaryNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    ExprNode* getCondition() const;
    ExprNode* getThen() const;
    ExprNode* getElse() const;

private:
    /** The condition to test for. */
    ExprNode* condition;
    /** The expression when the condition is true. */
    ExprNode* thenCase;
    /** The expression when the condition is false. */
    ExprNode* elseCase;
};

/**
 * Represents a function call, e.g.\ `f(x: 1)`.
 */
class CallNode : public ExprNode
{
public:
    /**
     * Creates a CallNode.
     *
     * @param location Where this CallNode was found in the source.
     * @param callee The function to call.
     * @param args The arguments to pass to the callee.
     */
    CallNode(Location location, ExprNode* callee, std::vector<ArgNode*> args);
    virtual ~CallNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    ExprNode* getCallee() const;
    llvm::ArrayRef<ArgNode*> getArgs() const;
    size_t getNumArgs() const;
    ArgNode* getArg(size_t i) const;

private:
    /** The function to call. */
    ExprNode* callee;
    /** The arguments to pass to the callee. */
    std::vector<ArgNode*> args;
};

/**
 * Represents a function call argument, e.g.\ `x: 1`.
 */
class ArgNode : public Node
{
public:
    /**
     * Creates an ArgNode.
     *
     * @param location Where this ArgNode was found in the source.
     * @param name The name of the argument. Used for name parameters (WIP).
     * @param value The value of the argument.
     */
    ArgNode(Location location, llvm::StringRef name, ExprNode* value);
    ~ArgNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    llvm::StringRef getName() const;
    ExprNode* getValue() const;

private:
    /** The name of the argument. */
    llvm::StringRef name;
    /** The value of the argument. */
    ExprNode* value;
};

#endif // NODE_HPP
