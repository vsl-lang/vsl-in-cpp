#include "node.hpp"
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

std::ostream& operator<<(std::ostream& os, const Node& ast)
{
    return os << ast.toString();
}

Node::Node(Node::Type type, size_t pos)
    : type{ type }, pos{ pos }
{
}

Node::~Node()
{
}

Node::Type Node::getType() const
{
    return type;
}

size_t Node::getPos() const
{
    return pos;
}

BlockNode::BlockNode(std::vector<std::unique_ptr<Node>>&& statements,
        size_t pos)
    : Node{ Node::BLOCK, pos }, statements{ std::move(statements) }
{
}

BlockNode::~BlockNode()
{
}

std::string BlockNode::toString() const
{
    std::string s = "Block { statements: [";
    if (!statements.empty())
    {
        s += ' ';
        s += statements[0]->toString();
        for (size_t i = 1; i < statements.size(); ++i)
        {
            s += ", ";
            s += statements[i]->toString();
        }
        s += ' ';
    }
    s += "] }";
    return s;
}

ArgNode::ArgNode(std::string name, std::unique_ptr<ExprNode> value, size_t pos)
    : Node{ Node::ARG, pos }, name{ std::move(name) }, value{ std::move(value) }
{
}

ArgNode::~ArgNode()
{
}

std::string ArgNode::toString() const
{
    std::string s = "Arg { name: ";
    s += name;
    s += ", value: ";
    s += value->toString();
    s += " }";
    return s;
}

const std::string& ArgNode::getName() const
{
    return name;
}

const ExprNode& ArgNode::getValue() const
{
    return *value;
}

ExprNode::ExprNode(Node::Type type, size_t pos)
    : Node{ type, pos }
{
}

IdentExprNode::IdentExprNode(std::string name, size_t pos)
    : ExprNode{ Node::ID_EXPR, pos }, name{ std::move(name) }
{
}

IdentExprNode::~IdentExprNode()
{
}

std::string IdentExprNode::toString() const
{
    std::string s = "Ident { name: ";
    s += name;
    s += " }";
    return s;
}

const std::string IdentExprNode::getName() const
{
    return name;
}

NumberExprNode::NumberExprNode(long value, size_t pos)
    : ExprNode{ Node::NUMBER_EXPR, pos }, value{ value }
{
}

NumberExprNode::~NumberExprNode()
{
}

std::string NumberExprNode::toString() const
{
    std::string s = "Number { value: ";
    s += std::to_string(value);
    s += " }";
    return s;
}

long NumberExprNode::getValue() const
{
    return value;
}

UnaryExprNode::UnaryExprNode(Token::Type op, std::unique_ptr<ExprNode> expr,
    size_t pos)
    : ExprNode{ Node::UNARY_EXPR, pos }, op{ op }, expr{ std::move(expr) }
{
}

UnaryExprNode::~UnaryExprNode()
{
}

std::string UnaryExprNode::toString() const
{
    std::string s = "Unary { op: ";
    s += Token::typeToString(op);
    s += ", expr: ";
    s += expr->toString();
    s += " }";
    return s;
}

Token::Type UnaryExprNode::getOp() const
{
    return op;
}

const ExprNode& UnaryExprNode::getExpr() const
{
    return *expr;
}

BinaryExprNode::BinaryExprNode(Token::Type op, std::unique_ptr<ExprNode> left,
    std::unique_ptr<ExprNode> right, size_t pos)
    : ExprNode{ Node::BINARY_EXPR, pos }, op{ op }, left{ std::move(left) },
        right{ std::move(right) }
{
}

BinaryExprNode::~BinaryExprNode()
{
}

std::string BinaryExprNode::toString() const
{
    std::string s = "Binary { op: ";
    s += Token::typeToString(op);
    s += ", left: ";
    s += left->toString();
    s += ", right: ";
    s += right->toString();
    s += " }";
    return s;
}

Token::Type BinaryExprNode::getOp() const
{
    return op;
}

const ExprNode& BinaryExprNode::getLeft() const
{
    return *left;
}

const ExprNode& BinaryExprNode::getRight() const
{
    return *right;
}

CallExprNode::CallExprNode(std::unique_ptr<ExprNode> callee,
    std::vector<std::unique_ptr<ArgNode>> args, size_t pos)
    : ExprNode{ Node::CALL_EXPR, pos }, callee{ std::move(callee) },
    args{ std::move(args) }
{
}

CallExprNode::~CallExprNode()
{
}

std::string CallExprNode::toString() const
{
    std::string s = "Call { callee: ";
    s += callee->toString();
    s += ", args: [";
    if (!args.empty())
    {
        s += ' ';
        s += args[0]->toString();
        for (size_t i = 1; i < args.size(); ++i)
        {
            s += ", ";
            s += args[i]->toString();
        }
        s += ' ';
    }
    s += "] }";
    return s;
}

const ExprNode& CallExprNode::getCallee() const
{
    return *callee;
}

size_t CallExprNode::getArgCount() const
{
    return args.size();
}

const ArgNode& CallExprNode::getArg(size_t arg) const
{
    return *args[arg];
}
