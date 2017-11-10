#include "ast/node.hpp"
#include <utility>

std::ostream& operator<<(std::ostream& os, const Node& ast)
{
    return os << ast.toString();
}

Node::Node(Kind kind, Location location, const Type* type)
    : kind{ kind }, location{ location }, type{ type }
{
}

Node::~Node()
{
}

ErrorNode::ErrorNode(Location location)
    : Node{ Node::ERROR, location }
{
}

void ErrorNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string ErrorNode::toString() const
{
    return "Error {}";
}

EmptyNode::EmptyNode(Location location)
    : Node{ Node::EMPTY, location }
{
}

void EmptyNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string EmptyNode::toString() const
{
    return "Empty {}";
}

BlockNode::BlockNode(std::vector<std::unique_ptr<Node>> statements,
    Location location)
    : Node{ Node::BLOCK, location }, statements{ std::move(statements) }
{
}

void BlockNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
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

ConditionalNode::ConditionalNode(std::unique_ptr<Node> condition,
    std::unique_ptr<Node> thenCase, std::unique_ptr<Node> elseCase,
    Location location)
    : Node{ Node::CONDITIONAL, location },
    condition{ std::move(condition) },
    thenCase{ std::move(thenCase) }, elseCase{ std::move(elseCase) }
{
}

void ConditionalNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string ConditionalNode::toString() const
{
    std::string s = "Conditional { condition: ";
    s += condition->toString();
    s += ", thenCase: ";
    s += thenCase->toString();
    s += ", elseCase: ";
    s += elseCase->toString();
    s += " }";
    return s;
}

AssignmentNode::AssignmentNode(llvm::StringRef name, const Type* type,
    std::unique_ptr<Node> value, Qualifiers qualifiers, Location location)
    : Node{ Node::ASSIGNMENT, location, type }, name{ name },
    value{ std::move(value) }, qualifiers{ qualifiers }
{
}

void AssignmentNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string AssignmentNode::toString() const
{
    std::string s = "Assignment { name: ";
    s += name;
    s += ", type: ";
    s += type->toString();
    s += ", value: ";
    if (value == nullptr)
    {
        s += "null";
    }
    else
    {
        s += value->toString();
    }
    s += ", qualifiers: [ ";
    if (qualifiers & CONST)
    {
        s += "const";
    }
    else
    {
        s += "nonConst";
    }
    s += " ] }";
    return s;
}

FunctionNode::FunctionNode(llvm::StringRef name, std::vector<Param> params,
    const Type* returnType, std::unique_ptr<Node> body, Location location)
    : Node{ Node::FUNCTION, location }, name{ name },
    params{ std::move(params) }, returnType{ returnType },
    body{ std::move(body) }
{
}

void FunctionNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string FunctionNode::toString() const
{
    std::string s = "Function { name: ";
    s += name.str();
    s += ", params: [";
    if (!params.empty())
    {
        s += ' ';
        s += params[0].toString();
        for (size_t i = 1; i < params.size(); ++i)
        {
            s += ", ";
            s += params[i].toString();
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

FunctionNode::Param::Param(llvm::StringRef name, const Type* type,
    Location Location)
    : name{ name }, type{ type }, location{ location }
{
}

std::string FunctionNode::Param::toString() const
{
    std::string s = name.str();
    s += ": ";
    s += type->toString();
    return s;
}

ReturnNode::ReturnNode(std::unique_ptr<Node> value, Location location)
    : Node{ Node::RETURN, location }, value{ std::move(value) }
{
}

void ReturnNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string ReturnNode::toString() const
{
    std::string s = "Return { value: ";
    s += value->toString();
    s += " }";
    return s;
}

ExprNode::ExprNode(Node::Kind kind, Location location)
    : Node{ kind, location }
{
}

IdentExprNode::IdentExprNode(llvm::StringRef name, Location location)
    : ExprNode{ Node::ID_EXPR, location }, name{ name }
{
}

void IdentExprNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string IdentExprNode::toString() const
{
    std::string s = "Ident { name: ";
    s += name;
    s += " }";
    return s;
}

IntExprNode::IntExprNode(llvm::APInt value, Location location)
    : ExprNode{ Node::NUMBER_EXPR, location }, value{ std::move(value) }
{
}

void IntExprNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string IntExprNode::toString() const
{
    std::string s = "Integer { value: ";
    s += value.toString(10, false);
    s += " }";
    return s;
}

UnaryExprNode::UnaryExprNode(TokenKind op, std::unique_ptr<Node> expr,
    Location location)
    : ExprNode{ Node::UNARY_EXPR, location }, op{ op },
    expr{ std::move(expr) }
{
}

void UnaryExprNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string UnaryExprNode::toString() const
{
    std::string s = "Unary { op: ";
    s += getTokenKindName(op);
    s += ", expr: ";
    s += expr->toString();
    s += " }";
    return s;
}

BinaryExprNode::BinaryExprNode(TokenKind op, std::unique_ptr<Node> left,
    std::unique_ptr<Node> right, Location location)
    : ExprNode{ Node::BINARY_EXPR, location }, op{ op },
    left{ std::move(left) }, right{ std::move(right) }
{
}

void BinaryExprNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
}

std::string BinaryExprNode::toString() const
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

CallExprNode::CallExprNode(std::unique_ptr<Node> callee,
    std::vector<std::unique_ptr<Node>> args, Location location)
    : ExprNode{ Node::CALL_EXPR, location }, callee{ std::move(callee) },
    args{ std::move(args) }
{
}

void CallExprNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
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

ArgNode::ArgNode(llvm::StringRef name, std::unique_ptr<Node> value,
    Location location)
    : Node{ Node::ARG, location }, name{ name },
    value{ std::move(value) }
{
}

void ArgNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visit(*this);
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
