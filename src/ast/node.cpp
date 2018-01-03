#include "ast/node.hpp"

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

bool Node::isExpr() const
{
    return false;
}

EmptyNode::EmptyNode(Location location)
    : Node{ Node::EMPTY, location }
{
}

void EmptyNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitEmpty(*this);
}

BlockNode::BlockNode(Location location, std::vector<Node*> statements)
    : Node{ Node::BLOCK, location }, statements{ std::move(statements) }
{
}

void BlockNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitBlock(*this);
}

llvm::ArrayRef<Node*> BlockNode::getStatements() const
{
    return statements;
}

IfNode::IfNode(Location location, ExprNode* condition, Node* thenCase,
    Node* elseCase)
    : Node{ Node::IF, location }, condition{ condition }, thenCase{ thenCase },
    elseCase{ elseCase }
{
}

void IfNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitIf(*this);
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

VariableNode::VariableNode(Location location, llvm::StringRef name,
    const Type* type, ExprNode* init, bool constness)
    : Node{ Node::VARIABLE, location }, name{ name }, type{ type },
    init{ init }, constness{ constness }
{
}

void VariableNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitVariable(*this);
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

FuncInterfaceNode::FuncInterfaceNode(Node::Kind kind, Location location,
    llvm::StringRef name, std::vector<ParamNode*> params,
    const Type* returnType, const FunctionType* ft)
    : Node{ kind, location }, name{ name }, params{ std::move(params) },
    returnType{ returnType }, ft{ ft }
{
}

llvm::StringRef FuncInterfaceNode::getName() const
{
    return name;
}

llvm::ArrayRef<ParamNode*> FuncInterfaceNode::getParams() const
{
    return params;
}

size_t FuncInterfaceNode::getNumParams() const
{
    return params.size();
}

ParamNode* FuncInterfaceNode::getParam(size_t i) const
{
    return params[i];
}

const Type* FuncInterfaceNode::getReturnType() const
{
    return returnType;
}

const FunctionType* FuncInterfaceNode::getFuncType() const
{
    return ft;
}

FunctionNode::FunctionNode(Location location, llvm::StringRef name,
    std::vector<ParamNode*> params, const Type* returnType,
    const FunctionType* ft, Node* body)
    : FuncInterfaceNode{ Node::FUNCTION, location, name, std::move(params),
        returnType, ft }, body{ body }, alreadyDefined{ false }
{
}

void FunctionNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitFunction(*this);
}

bool FunctionNode::isAlreadyDefined() const
{
    return alreadyDefined;
}

void FunctionNode::setAlreadyDefined(bool alreadyDefined)
{
    this->alreadyDefined = alreadyDefined;
}

Node* FunctionNode::getBody() const
{
    return body;
}

ExtFuncNode::ExtFuncNode(Location location, llvm::StringRef name,
    std::vector<ParamNode*> params, const Type* returnType,
    const FunctionType* ft, llvm::StringRef alias)
    : FuncInterfaceNode{ Node::EXTFUNC, location, name, std::move(params),
        returnType, ft }, alias{ alias }
{
}

void ExtFuncNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitExtFunc(*this);
}

llvm::StringRef ExtFuncNode::getAlias() const
{
    return alias;
}

ParamNode::ParamNode(Location location, llvm::StringRef name, const Type* type)
    : Node{ Node::PARAM, location }, name{ name }, type{ type }
{
}

void ParamNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitParam(*this);
}

llvm::StringRef ParamNode::getName() const
{
    return name;
}

const Type* ParamNode::getType() const
{
    return type;
}

ReturnNode::ReturnNode(Location location, ExprNode* value)
    : Node{ Node::RETURN, location }, value{ std::move(value) }
{
}

void ReturnNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitReturn(*this);
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

bool ExprNode::isExpr() const
{
    return true;
}

const Type* ExprNode::getType() const
{
    return type;
}

void ExprNode::setType(const Type* t)
{
    type = t;
}

IdentNode::IdentNode(Location location, llvm::StringRef name)
    : ExprNode{ Node::IDENT, location }, name{ name }
{
}

void IdentNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitIdent(*this);
}

llvm::StringRef IdentNode::getName() const
{
    return name;
}

LiteralNode::LiteralNode(Location location, llvm::APInt value)
    : ExprNode{ Node::LITERAL, location }, value{ std::move(value) }
{
}

void LiteralNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitLiteral(*this);
}

llvm::APInt LiteralNode::getValue() const
{
    return value;
}

UnaryNode::UnaryNode(Location location, TokenKind op, ExprNode* expr)
    : ExprNode{ Node::UNARY, location }, op{ op }, expr{ expr }
{
}

void UnaryNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitUnary(*this);
}

TokenKind UnaryNode::getOp() const
{
    return op;
}

ExprNode* UnaryNode::getExpr() const
{
    return expr;
}

BinaryNode::BinaryNode(Location location, TokenKind op, ExprNode* left,
    ExprNode* right)
    : ExprNode{ Node::BINARY, location }, op{ op }, left{ left }, right{ right }
{
}

void BinaryNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitBinary(*this);
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

TernaryNode::TernaryNode(Location location, ExprNode* condition,
    ExprNode* thenCase, ExprNode* elseCase)
    : ExprNode{ Node::TERNARY, location }, condition{ condition },
    thenCase{ thenCase }, elseCase{ elseCase }
{
}

void TernaryNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitTernary(*this);
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

CallNode::CallNode(Location location, ExprNode* callee,
    std::vector<ArgNode*> args)
    : ExprNode{ Node::CALL, location }, callee{ callee },
    args{ std::move(args) }
{
}

void CallNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitCall(*this);
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

ArgNode::ArgNode(Location location, llvm::StringRef name, ExprNode* value)
    : Node{ Node::ARG, location }, name{ name }, value{ value }
{
}

void ArgNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitArg(*this);
}

llvm::StringRef ArgNode::getName() const
{
    return name;
}

ExprNode* ArgNode::getValue() const
{
    return value;
}
