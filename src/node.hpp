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
class ParamNode;
class TypeNode;
class ExprNode;
class IdentExprNode;
class NumberExprNode;
class UnaryExprNode;
class BinaryExprNode;
class CallExprNode;
class ArgNode;

#include "location.hpp"
#include "nodevisitor.hpp"
#include "token.hpp"
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

std::ostream& operator<<(std::ostream& os, const Node& ast);

class Node
{
public:
    enum Type
    {
        ERROR,
        EMPTY,
        BLOCK,
        CONDITIONAL,
        ASSIGNMENT,
        FUNCTION,
        RETURN,
        PARAM,
        TYPE,
        ID_EXPR,
        NUMBER_EXPR,
        UNARY_EXPR,
        BINARY_EXPR,
        CALL_EXPR,
        ARG
    };
    Node(Type nodeType, Location location);
    virtual ~Node() = 0;
    virtual void accept(NodeVisitor& nodeVisitor) = 0;
    virtual std::string toString() const = 0;
    Type getNodeType() const;
    Location getLocation() const;

private:
    Type nodeType;
    Location location;
};

class ErrorNode : public Node
{
public:
    ErrorNode(Location location);
    virtual ~ErrorNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
};

class EmptyNode : public Node
{
public:
    EmptyNode(Location location);
    virtual ~EmptyNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
};

class BlockNode : public Node
{
public:
    BlockNode(std::vector<std::unique_ptr<Node>> statements, Location location);
    virtual ~BlockNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;

private:
    std::vector<std::unique_ptr<Node>> statements;
};

class ConditionalNode : public Node
{
public:
    ConditionalNode(std::unique_ptr<Node> condition,
        std::unique_ptr<Node> thenCase, std::unique_ptr<Node> elseCase,
        Location location);
    virtual ~ConditionalNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;

private:
    std::unique_ptr<Node> condition;
    std::unique_ptr<Node> thenCase;
    std::unique_ptr<Node> elseCase;
};

class AssignmentNode : public Node
{
public:
    enum Qualifiers
    {
        NONCONST = 0,
        CONST = 1
    };
    AssignmentNode(std::string name, std::unique_ptr<Node> type,
        std::unique_ptr<Node> value, Qualifiers qualifiers, Location location);
    virtual ~AssignmentNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;

private:
    std::string name;
    std::unique_ptr<Node> type;
    std::unique_ptr<Node> value;
    Qualifiers qualifiers;
};

class FunctionNode : public Node
{
public:
    FunctionNode(std::string name, std::vector<std::unique_ptr<Node>> params,
        std::unique_ptr<Node> returnType, std::unique_ptr<Node> body,
        Location location);
    virtual ~FunctionNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;

private:
    std::string name;
    std::vector<std::unique_ptr<Node>> params;
    std::unique_ptr<Node> returnType;
    std::unique_ptr<Node> body;
};

class ReturnNode : public Node
{
public:
    ReturnNode(std::unique_ptr<Node> value, Location location);
    virtual ~ReturnNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;

private:
    std::unique_ptr<Node> value;
};

class ParamNode : public Node
{
public:
    ParamNode(std::string name, std::unique_ptr<Node> type, Location location);
    virtual ~ParamNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;

private:
    std::string name;
    std::unique_ptr<Node> type;
};

class TypeNode : public Node
{
public:
    TypeNode(std::string name, Location location);
    virtual ~TypeNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;

private:
    std::string name;
};

class ExprNode : public Node
{
public:
    ExprNode(Node::Type type, Location location);
};

class IdentExprNode : public ExprNode
{
public:
    IdentExprNode(std::string name, Location location);
    virtual ~IdentExprNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    const std::string& getName() const;

private:
    std::string name;
};

class NumberExprNode : public ExprNode
{
public:
    NumberExprNode(long value, Location location);
    virtual ~NumberExprNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    long getValue() const;

private:
    long value;
};

class UnaryExprNode : public ExprNode
{
public:
    UnaryExprNode(Token::Type op, std::unique_ptr<Node> expr,
        Location location);
    virtual ~UnaryExprNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    Token::Type getOp() const;
    const Node& getExpr() const;

private:
    Token::Type op;
    std::unique_ptr<Node> expr;
};

class BinaryExprNode : public ExprNode
{
public:
    BinaryExprNode(Token::Type op, std::unique_ptr<Node> left,
        std::unique_ptr<Node> right, Location location);
    virtual ~BinaryExprNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    Token::Type getOp() const;
    const Node& getLeft() const;
    const Node& getRight() const;

private:
    Token::Type op;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
};

class CallExprNode : public ExprNode
{
public:
    CallExprNode(std::unique_ptr<Node> callee,
        std::vector<std::unique_ptr<Node>> args, Location location);
    virtual ~CallExprNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    const Node& getCallee() const;
    size_t getArgCount() const;
    const Node& getArg(size_t arg) const;

private:
    std::unique_ptr<Node> callee;
    std::vector<std::unique_ptr<Node>> args;
};

class ArgNode : public Node
{
public:
    ArgNode(std::string name, std::unique_ptr<Node> value, Location location);
    ~ArgNode() override;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    const std::string& getName() const;
    const Node& getValue() const;

private:
    std::string name;
    std::unique_ptr<Node> value;
};

#endif // NODE_HPP
