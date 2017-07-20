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

#include "location.hpp"
#include "nodevisitor.hpp"
#include "token.hpp"
#include "type.hpp"
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

std::ostream& operator<<(std::ostream& os, const Node& ast);

class Node
{
public:
    enum Kind
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
    Node(Kind kind, Location location, std::unique_ptr<Type> type = nullptr);
    virtual ~Node() = 0;
    virtual void accept(NodeVisitor& nodeVisitor) = 0;
    virtual std::string toString() const = 0;
    std::unique_ptr<Type> type;
    Kind kind;
    Location location;
};

class ErrorNode : public Node
{
public:
    ErrorNode(Location location);
    virtual ~ErrorNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
};

class EmptyNode : public Node
{
public:
    EmptyNode(Location location);
    virtual ~EmptyNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
};

class BlockNode : public Node
{
public:
    BlockNode(std::vector<std::unique_ptr<Node>> statements, Location location);
    virtual ~BlockNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    std::vector<std::unique_ptr<Node>> statements;
};

class ConditionalNode : public Node
{
public:
    ConditionalNode(std::unique_ptr<Node> condition,
        std::unique_ptr<Node> thenCase, std::unique_ptr<Node> elseCase,
        Location location);
    virtual ~ConditionalNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
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
    AssignmentNode(std::string name, std::unique_ptr<Type> type,
        std::unique_ptr<Node> value, Qualifiers qualifiers, Location location);
    virtual ~AssignmentNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    std::string name;
    std::unique_ptr<Node> value;
    Qualifiers qualifiers;
};

class FunctionNode : public Node
{
public:
    struct ParamName
    {
        ParamName() = default;
        ParamName(std::string str, Location location);
        std::string str;
        Location location;
    };
    struct Param
    {
        ParamName name;
        std::unique_ptr<Type> type;
    };
    FunctionNode(std::string name, std::vector<Param> params,
        std::unique_ptr<Type> returnType, std::unique_ptr<Node> body,
        Location location);
    virtual ~FunctionNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    static std::string paramToString(const std::string& name, const Type& type);
    std::string name;
    std::vector<ParamName> paramNames;
    std::unique_ptr<Node> body;
};

class ReturnNode : public Node
{
public:
    ReturnNode(std::unique_ptr<Node> value, Location location);
    virtual ~ReturnNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    std::unique_ptr<Node> value;
};

class ExprNode : public Node
{
public:
    ExprNode(Node::Kind kind, Location location);
};

class IdentExprNode : public ExprNode
{
public:
    IdentExprNode(std::string name, Location location);
    virtual ~IdentExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    std::string name;
};

class NumberExprNode : public ExprNode
{
public:
    NumberExprNode(long value, Location location);
    virtual ~NumberExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    long value;
};

class UnaryExprNode : public ExprNode
{
public:
    UnaryExprNode(Token::Kind op, std::unique_ptr<Node> expr,
        Location location);
    virtual ~UnaryExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    Token::Kind op;
    std::unique_ptr<Node> expr;
};

class BinaryExprNode : public ExprNode
{
public:
    BinaryExprNode(Token::Kind op, std::unique_ptr<Node> left,
        std::unique_ptr<Node> right, Location location);
    virtual ~BinaryExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    Token::Kind op;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
};

class CallExprNode : public ExprNode
{
public:
    CallExprNode(std::unique_ptr<Node> callee,
        std::vector<std::unique_ptr<Node>> args, Location location);
    virtual ~CallExprNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    std::unique_ptr<Node> callee;
    std::vector<std::unique_ptr<Node>> args;
};

class ArgNode : public Node
{
public:
    ArgNode(std::string name, std::unique_ptr<Node> value, Location location);
    ~ArgNode() override = default;
    virtual void accept(NodeVisitor& nodeVisitor) override;
    virtual std::string toString() const override;
    std::string name;
    std::unique_ptr<Node> value;
};

#endif // NODE_HPP
