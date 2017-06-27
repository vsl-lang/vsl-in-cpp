#ifndef NODE_HPP
#define NODE_HPP

#include "token.hpp"
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

class Node;
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

std::ostream& operator<<(std::ostream& os, const Node& ast);

class Node
{
public:
    enum Type
    {
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
    Node(Type nodeType, size_t pos);
    virtual ~Node() = 0;
    virtual std::string toString() const = 0;
    Type getNodeType() const;
    size_t getPos() const;

private:
    Type nodeType;
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

class ConditionalNode : public Node
{
public:
    ConditionalNode(std::unique_ptr<ExprNode> condition,
        std::unique_ptr<Node> thenCase, std::unique_ptr<Node> elseCase,
        size_t pos);
    virtual ~ConditionalNode() override;
    virtual std::string toString() const override;

private:
    std::unique_ptr<ExprNode> condition;
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
    AssignmentNode(std::string name, std::unique_ptr<TypeNode> type,
        std::unique_ptr<ExprNode> value, Qualifiers qualifiers, size_t pos);
    virtual ~AssignmentNode() override;
    virtual std::string toString() const override;

private:
    std::string name;
    std::unique_ptr<TypeNode> type;
    std::unique_ptr<ExprNode> value;
    Qualifiers qualifiers;
};

class FunctionNode : public Node
{
public:
    FunctionNode(std::string name,
        std::vector<std::unique_ptr<ParamNode>> params,
        std::unique_ptr<TypeNode> returnType, std::unique_ptr<BlockNode> body,
        size_t pos);
    virtual ~FunctionNode() override;
    virtual std::string toString() const override;

private:
    std::string name;
    std::vector<std::unique_ptr<ParamNode>> params;
    std::unique_ptr<TypeNode> returnType;
    std::unique_ptr<BlockNode> body;
};

class ReturnNode : public Node
{
public:
    ReturnNode(std::unique_ptr<ExprNode> value, size_t pos);
    virtual ~ReturnNode() override;
    virtual std::string toString() const override;

private:
    std::unique_ptr<ExprNode> value;
};

class ParamNode : public Node
{
public:
    ParamNode(std::string name, std::unique_ptr<TypeNode> type, size_t pos);
    virtual ~ParamNode() override;
    virtual std::string toString() const override;

private:
    std::string name;
    std::unique_ptr<TypeNode> type;
};

class TypeNode : public Node
{
public:
    TypeNode(std::string name, size_t pos);
    virtual ~TypeNode() override;
    virtual std::string toString() const override;

private:
    std::string name;
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
    const std::string& getName() const;

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

#endif // NODE_HPP
