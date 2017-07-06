#include "node.hpp"
#include <utility>

std::ostream& operator<<(std::ostream& os, const Node& ast)
{
    return os << ast.toString();
}

Node::Node(Type nodeType, Location location)
    : nodeType{ nodeType }, location{ location }
{
}

Node::~Node()
{
}

Node::Type Node::getNodeType() const
{
    return nodeType;
}

Location Node::getLocation() const
{
    return location;
}

ErrorNode::ErrorNode(Location location)
    : Node{ Node::ERROR, location }
{
}

ErrorNode::~ErrorNode()
{
}

std::string ErrorNode::toString() const
{
    return "Error {}";
}

EmptyNode::EmptyNode(Location location)
    : Node{ Node::EMPTY, location }
{
}

EmptyNode::~EmptyNode()
{
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

ConditionalNode::ConditionalNode(std::unique_ptr<Node> condition,
    std::unique_ptr<Node> thenCase, std::unique_ptr<Node> elseCase,
    Location location)
    : Node{ Node::CONDITIONAL, location },
    condition{ std::move(condition) },
    thenCase{ std::move(thenCase) }, elseCase{ std::move(elseCase) }
{
}

ConditionalNode::~ConditionalNode()
{
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

AssignmentNode::AssignmentNode(std::string name, std::unique_ptr<Node> type,
    std::unique_ptr<Node> value, Qualifiers qualifiers, Location location)
    : Node{ Node::ASSIGNMENT, location }, name{ std::move(name) },
    type{ std::move(type) }, value{ std::move(value) },
    qualifiers{ qualifiers }
{
}

AssignmentNode::~AssignmentNode()
{
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

FunctionNode::FunctionNode(std::string name,
    std::vector<std::unique_ptr<Node>> params, std::unique_ptr<Node> returnType,
    std::unique_ptr<Node> body, Location location)
    : Node{ Node::FUNCTION, location }, name{ std::move(name) },
    params{ std::move(params) }, returnType{ std::move(returnType) },
    body{ std::move(body) }
{
}

FunctionNode::~FunctionNode()
{
}

std::string FunctionNode::toString() const
{
    std::string s = "Function { name: ";
    s += name;
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

ReturnNode::ReturnNode(std::unique_ptr<Node> value, Location location)
    : Node{ Node::RETURN, location }, value{ std::move(value) }
{
}

ReturnNode::~ReturnNode()
{
}

std::string ReturnNode::toString() const
{
    std::string s = "Return { value: ";
    s += value->toString();
    s += " }";
    return s;
}

ParamNode::ParamNode(std::string name, std::unique_ptr<Node> type,
    Location location)
    : Node{ Node::PARAM, location }, name{ std::move(name) },
    type{ std::move(type) }
{
}

ParamNode::~ParamNode()
{
}

std::string ParamNode::toString() const
{
    std::string s = "Param { name: ";
    s += name;
    s += ", type: ";
    s += type->toString();
    s += " }";
    return s;
}

TypeNode::TypeNode(std::string name, Location location)
    : Node{ Node::TYPE, location }, name{ std::move(name) }
{
}

TypeNode::~TypeNode()
{
}

std::string TypeNode::toString() const
{
    std::string s = "Type { name: ";
    s += name;
    s += " }";
    return s;
}

ExprNode::ExprNode(Node::Type type, Location location)
    : Node{ type, location }
{
}

IdentExprNode::IdentExprNode(std::string name, Location location)
    : ExprNode{ Node::ID_EXPR, location }, name{ std::move(name) }
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

const std::string& IdentExprNode::getName() const
{
    return name;
}

NumberExprNode::NumberExprNode(long value, Location location)
    : ExprNode{ Node::NUMBER_EXPR, location }, value{ value }
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

UnaryExprNode::UnaryExprNode(Token::Type op, std::unique_ptr<Node> expr,
    Location location)
    : ExprNode{ Node::UNARY_EXPR, location }, op{ op },
    expr{ std::move(expr) }
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

const Node& UnaryExprNode::getExpr() const
{
    return *expr;
}

BinaryExprNode::BinaryExprNode(Token::Type op, std::unique_ptr<Node> left,
    std::unique_ptr<Node> right, Location location)
    : ExprNode{ Node::BINARY_EXPR, location }, op{ op },
    left{ std::move(left) }, right{ std::move(right) }
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

const Node& BinaryExprNode::getLeft() const
{
    return *left;
}

const Node& BinaryExprNode::getRight() const
{
    return *right;
}

CallExprNode::CallExprNode(std::unique_ptr<Node> callee,
    std::vector<std::unique_ptr<Node>> args, Location location)
    : ExprNode{ Node::CALL_EXPR, location }, callee{ std::move(callee) },
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

const Node& CallExprNode::getCallee() const
{
    return *callee;
}

size_t CallExprNode::getArgCount() const
{
    return args.size();
}

const Node& CallExprNode::getArg(size_t arg) const
{
    return *args[arg];
}

ArgNode::ArgNode(std::string name, std::unique_ptr<Node> value,
    Location location)
    : Node{ Node::ARG, location }, name{ std::move(name) },
    value{ std::move(value) }
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

const Node& ArgNode::getValue() const
{
    return *value;
}
