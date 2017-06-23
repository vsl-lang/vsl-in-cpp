#ifndef NODE_HPP
#define NODE_HPP

#include "token.hpp"
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

class Node;

std::ostream& operator<<(std::ostream& os, const Node& ast);

class Node
{
public:
    enum Type
    {
        EMPTY,
        BLOCK,
        ARG,
        ID_EXPR,
        NUMBER_EXPR,
        UNARY_EXPR,
        BINARY_EXPR,
        CALL_EXPR
    };
    Node(Type type, size_t pos);
    virtual ~Node() = 0;
    virtual std::string toString() const = 0;
    Type getType() const;
    size_t getPos() const;

private:
    Type type;
    size_t pos;
};

class EmptyNode : public Node
{
public:
    EmptyNode(size_t pos);
    virtual ~EmptyNode() override;
    virtual std::string toString() const override;
};

class BlockNode : public Node
{
public:
    BlockNode(std::vector<std::unique_ptr<Node>> statements, size_t pos);
    virtual ~BlockNode() override;
    virtual std::string toString() const override;

private:
    std::vector<std::unique_ptr<Node>> statements;
};

class ExprNode;

class ArgNode : public Node
{
public:
    ArgNode(std::string name, std::unique_ptr<ExprNode> value, size_t pos);
    ~ArgNode() override;
    virtual std::string toString() const override;
    const std::string& getName() const;
    const ExprNode& getValue() const;

private:
    std::string name;
    std::unique_ptr<ExprNode> value;
};

class ExprNode : public Node
{
public:
    ExprNode(Node::Type type, size_t pos);
};

class IdentExprNode : public ExprNode
{
public:
    IdentExprNode(std::string name, size_t pos);
    virtual ~IdentExprNode() override;
    virtual std::string toString() const override;
    const std::string getName() const;

private:
    std::string name;
};

class NumberExprNode : public ExprNode
{
public:
    NumberExprNode(long value, size_t pos);
    virtual ~NumberExprNode() override;
    virtual std::string toString() const override;
    long getValue() const;

private:
    long value;
};

class UnaryExprNode : public ExprNode
{
public:
    UnaryExprNode(Token::Type op, std::unique_ptr<ExprNode> expr, size_t pos);
    virtual ~UnaryExprNode() override;
    virtual std::string toString() const override;
    Token::Type getOp() const;
    const ExprNode& getExpr() const;

private:
    Token::Type op;
    std::unique_ptr<ExprNode> expr;
};

class BinaryExprNode : public ExprNode
{
public:
    BinaryExprNode(Token::Type op, std::unique_ptr<ExprNode> left,
        std::unique_ptr<ExprNode> right, size_t pos);
    virtual ~BinaryExprNode() override;
    virtual std::string toString() const override;
    Token::Type getOp() const;
    const ExprNode& getLeft() const;
    const ExprNode& getRight() const;

private:
    Token::Type op;
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
};

class CallExprNode : public ExprNode
{
public:
    CallExprNode(std::unique_ptr<ExprNode> callee,
        std::vector<std::unique_ptr<ArgNode>> args, size_t pos);
    virtual ~CallExprNode() override;
    virtual std::string toString() const override;
    const ExprNode& getCallee() const;
    size_t getArgCount() const;
    const ArgNode& getArg(size_t arg) const;

private:
    std::unique_ptr<ExprNode> callee;
    std::vector<std::unique_ptr<ArgNode>> args;
};

#endif // NODE_HPP
