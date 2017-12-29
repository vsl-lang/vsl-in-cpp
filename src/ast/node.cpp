#include "ast/node.hpp"

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

bool Node::is(Kind k) const
{
    return kind == k;
}

bool Node::isNot(Kind k) const
{
    return kind != k;
}

Location Node::getLoc() const
{
    return location;
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

llvm::ArrayRef<Node*> BlockNode::getStatements() const
{
    return statements;
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

ExprNode* IfNode::getCondition() const
{
    return condition;
}

Node* IfNode::getThen() const
{
    return thenCase;
}

Node* IfNode::getElse() const
{
    return elseCase;
}

VariableNode::VariableNode(llvm::StringRef name, const Type* type,
    ExprNode* init, bool constness, Location location)
    : Node{ Node::VARIABLE, location }, name{ name }, type{ type },
    init{ init }, constness{ constness }
{
}

void VariableNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitVariable(*this);
}

std::string VariableNode::toString() const
{
    std::string s = "Variable { name: ";
    s += name;
    s += ", type: ";
    s += type->toString();
    s += ", value: ";
    s += init->toString();
    s += ", const: ";
    s += constness ? "true" : "false";
    s += " }";
    return s;
}

llvm::StringRef VariableNode::getName() const
{
    return name;
}

const Type* VariableNode::getType() const
{
    return type;
}

ExprNode* VariableNode::getInit() const
{
    return init;
}

bool VariableNode::isConst() const
{
    return constness;
}

FunctionNode::FunctionNode(llvm::StringRef name, std::vector<ParamNode*> params,
    const Type* returnType, Node* body, const FunctionType* ft,
    Location location)
    : Node{ Node::FUNCTION, location }, name{ name },
    params{ std::move(params) }, returnType{ returnType }, body{ body },
    ft{ ft }, alreadyDefined{ false }
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

llvm::StringRef FunctionNode::getName() const
{
    return name;
}

llvm::ArrayRef<ParamNode*> FunctionNode::getParams() const
{
    return params;
}

size_t FunctionNode::getNumParams() const
{
    return params.size();
}

ParamNode* FunctionNode::getParam(size_t i) const
{
    return params[i];
}

const Type* FunctionNode::getReturnType() const
{
    return returnType;
}

Node* FunctionNode::getBody() const
{
    return body;
}

const FunctionType* FunctionNode::getFunctionType() const
{
    return ft;
}

bool FunctionNode::isAlreadyDefined() const
{
    return alreadyDefined;
}

void FunctionNode::setAlreadyDefined(bool alreadyDefined)
{
    this->alreadyDefined = alreadyDefined;
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

llvm::StringRef ParamNode::getName() const
{
    return name;
}

const Type* ParamNode::getType() const
{
    return type;
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
    std::string s;
    if (hasValue())
    {
        s = "Return { value: ";
        s += value->toString();
        s += " }";
    }
    else
    {
        s = "Return { value: <void> }";
    }
    return s;
}

bool ReturnNode::hasValue() const
{
    return value;
}

ExprNode* ReturnNode::getValue() const
{
    return value;
}

ExprNode::ExprNode(Node::Kind kind, Location location)
    : Node{ kind, location }, type{ nullptr }
{
}

const Type* ExprNode::getType() const
{
    return type;
}

void ExprNode::setType(const Type* t)
{
    type = t;
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

llvm::StringRef IdentNode::getName() const
{
    return name;
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

llvm::APInt LiteralNode::getValue() const
{
    return value;
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
    s += tokenKindDebugName(op);
    s += ", expr: ";
    s += expr->toString();
    s += " }";
    return s;
}

TokenKind UnaryNode::getOp() const
{
    return op;
}

ExprNode* UnaryNode::getExpr() const
{
    return expr;
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
    s += tokenKindDebugName(op);
    s += ", left: ";
    s += left->toString();
    s += ", right: ";
    s += right->toString();
    s += " }";
    return s;
}

TokenKind BinaryNode::getOp() const
{
    return op;
}

ExprNode* BinaryNode::getLhs() const
{
    return left;
}

ExprNode* BinaryNode::getRhs() const
{
    return right;
}

TernaryNode::TernaryNode(ExprNode* condition, ExprNode* thenCase,
    ExprNode* elseCase, Location location)
    : ExprNode{ Node::TERNARY, location }, condition{ condition },
    thenCase{ thenCase }, elseCase{ elseCase }
{
}

void TernaryNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitTernary(*this);
}

std::string TernaryNode::toString() const
{
    std::string s = "Ternary { condition: ";
    s += condition->toString();
    s += ", then: ";
    s += thenCase->toString();
    s += ", else: ";
    s += elseCase->toString();
    s += " }";
    return s;
}

ExprNode* TernaryNode::getCondition() const
{
    return condition;
}

ExprNode* TernaryNode::getThen() const
{
    return thenCase;
}

ExprNode* TernaryNode::getElse() const
{
    return elseCase;
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

ExprNode* CallNode::getCallee() const
{
    return callee;
}

llvm::ArrayRef<ArgNode*> CallNode::getArgs() const
{
    return args;
}

size_t CallNode::getNumArgs() const
{
    return args.size();
}

ArgNode* CallNode::getArg(size_t i) const
{
    return args[i];
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
    std::string s = name;
    s += ": ";
    s += value->toString();
    return s;
}

llvm::StringRef ArgNode::getName() const
{
    return name;
}

ExprNode* ArgNode::getValue() const
{
    return value;
}
