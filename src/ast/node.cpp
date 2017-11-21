#include "ast/node.hpp"
#include <utility>

std::ostream& operator<<(std::ostream& os, const Node& ast)
{
    return os << ast.toString();
}

Node::Node(Kind kind, Location location)
    : kind{ kind }, location{ location }
{
}

Node::~Node()
{
}

EmptyNode::EmptyNode(Location location)
    : Node{ Node::EMPTY, location }
{
}

void EmptyNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitEmpty(*this);
}

std::string EmptyNode::toString() const
{
    return "Empty {}";
}

BlockNode::BlockNode(std::vector<Node*> statements, Location location)
    : Node{ Node::BLOCK, location }, statements{ std::move(statements) }
{
}

void BlockNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitBlock(*this);
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

IfNode::IfNode(ExprNode* condition, Node* thenCase, Node* elseCase,
    Location location)
    : Node{ Node::IF, location }, condition{ condition }, thenCase{ thenCase },
    elseCase{ elseCase }
{
}

void IfNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitIf(*this);
}

std::string IfNode::toString() const
{
    std::string s = "If { condition: ";
    s += condition->toString();
    s += ", then: ";
    s += thenCase->toString();
    s += ", else: ";
    s += elseCase->toString();
    s += " }";
    return s;
}

VariableNode::VariableNode(llvm::StringRef name, const Type* type,
    ExprNode* value, bool isConst, Location location)
    : Node{ Node::VARIABLE, location }, name{ name }, type{ type },
    value{ value }, isConst{ isConst }
{
}

void VariableNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitVariable(*this);
}

std::string VariableNode::toString() const
{
    std::string s = "Assignment { name: ";
    s += name;
    s += ", type: ";
    s += type->toString();
    s += ", value: ";
    s += value ? value->toString() : "null";
    s += ", const: ";
    s += isConst ? "true" : "false";
    s += " }";
    return s;
}

FunctionNode::FunctionNode(llvm::StringRef name, std::vector<ParamNode*> params,
    const Type* returnType, Node* body, Location location)
    : Node{ Node::FUNCTION, location }, name{ name },
    params{ std::move(params) }, returnType{ returnType }, body{ body }
{
}

void FunctionNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitFunction(*this);
}

std::string FunctionNode::toString() const
{
    std::string s = "Function { name: ";
    s += name.str();
    s += ", params: [";
    if (!params.empty())
    {
        s += ' ';
        s += params[0]->toString();
        for (size_t i = 1; i < params.size(); ++i)
        {
            s += ", ";
            s += params[i]->toString();
        }
        s += ' ';
    }
    s += "], returnType: ";
    s += returnType->toString();
    s += ", body: ";
    s += body->toString();
    s += " ]";
    return s;
}

ParamNode::ParamNode(llvm::StringRef name, const Type* type, Location location)
    : Node{ Node::PARAM, location }, name{ name }, type{ type }
{
}

void ParamNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitParam(*this);
}

std::string ParamNode::toString() const
{
    std::string s = name.str();
    s += ": ";
    s += type->toString();
    return s;
}

ReturnNode::ReturnNode(ExprNode* value, Location location)
    : Node{ Node::RETURN, location }, value{ std::move(value) }
{
}

void ReturnNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitReturn(*this);
}

std::string ReturnNode::toString() const
{
    std::string s = "Return { value: ";
    s += value->toString();
    s += " }";
    return s;
}

ExprNode::ExprNode(Node::Kind kind, Location location)
    : Node{ kind, location }, type{ nullptr }
{
}

IdentNode::IdentNode(llvm::StringRef name, Location location)
    : ExprNode{ Node::IDENT, location }, name{ name }
{
}

void IdentNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitIdent(*this);
}

std::string IdentNode::toString() const
{
    std::string s = "Ident { name: ";
    s += name;
    s += " }";
    return s;
}

LiteralNode::LiteralNode(llvm::APInt value, Location location)
    : ExprNode{ Node::LITERAL, location }, value{ std::move(value) }
{
}

void LiteralNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitLiteral(*this);
}

std::string LiteralNode::toString() const
{
    std::string s = "Literal { value: ";
    s += value.toString(10, false);
    s += " }";
    return s;
}

UnaryNode::UnaryNode(TokenKind op, ExprNode* expr, Location location)
    : ExprNode{ Node::UNARY, location }, op{ op }, expr{ expr }
{
}

void UnaryNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitUnary(*this);
}

std::string UnaryNode::toString() const
{
    std::string s = "Unary { op: ";
    s += getTokenKindName(op);
    s += ", expr: ";
    s += expr->toString();
    s += " }";
    return s;
}

BinaryNode::BinaryNode(TokenKind op, ExprNode* left, ExprNode* right,
    Location location)
    : ExprNode{ Node::BINARY, location }, op{ op }, left{ left }, right{ right }
{
}

void BinaryNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitBinary(*this);
}

std::string BinaryNode::toString() const
{
    std::string s = "Binary { op: ";
    s += getTokenKindName(op);
    s += ", left: ";
    s += left->toString();
    s += ", right: ";
    s += right->toString();
    s += " }";
    return s;
}

CallNode::CallNode(ExprNode* callee, std::vector<ArgNode*> args,
    Location location)
    : ExprNode{ Node::CALL, location }, callee{ callee },
    args{ std::move(args) }
{
}

void CallNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitCall(*this);
}

std::string CallNode::toString() const
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

ArgNode::ArgNode(llvm::StringRef name, ExprNode* value, Location location)
    : Node{ Node::ARG, location }, name{ name }, value{ value }
{
}

void ArgNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitArg(*this);
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
