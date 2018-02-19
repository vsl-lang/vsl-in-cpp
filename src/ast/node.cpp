#include "ast/node.hpp"

Node::Node(Kind kind, Location location)
    : kind{ kind }, location{ location }
{
}

Node::~Node() = default;

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

DeclNode::DeclNode(Node::Kind kind, Location location, AccessMod access)
    : Node{ kind, location }, access{ access }
{
}

AccessMod DeclNode::getAccessMod() const
{
    return access;
}

FuncInterfaceNode::FuncInterfaceNode(Node::Kind kind, Location location,
    AccessMod access, llvm::StringRef name, std::vector<ParamNode*> params,
    const Type* returnType, const FunctionType* ft)
    : DeclNode{ kind, location, access }, name{ name },
    params{ std::move(params) }, returnType{ returnType }, ft{ ft }
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

ParamNode& FuncInterfaceNode::getParam(size_t i) const
{
    return *params[i];
}

const Type* FuncInterfaceNode::getReturnType() const
{
    return returnType;
}

const FunctionType* FuncInterfaceNode::getFuncType() const
{
    return ft;
}

FunctionNode::FunctionNode(Location location, AccessMod access,
    llvm::StringRef name, std::vector<ParamNode*> params,
    const Type* returnType, const FunctionType* ft, BlockNode& body)
    : FuncInterfaceNode{ Node::FUNCTION, location, access, name,
        std::move(params), returnType, ft }, body{ body },
    alreadyDefined{ false }
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

BlockNode& FunctionNode::getBody() const
{
    return body;
}

ExtFuncNode::ExtFuncNode(Location location, AccessMod access,
    llvm::StringRef name, std::vector<ParamNode*> params,
    const Type* returnType, const FunctionType* ft, llvm::StringRef alias)
    : FuncInterfaceNode{ Node::EXTFUNC, location, access, name,
        std::move(params), returnType, ft }, alias{ alias }
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

VariableNode::VariableNode(Location location, AccessMod access,
    llvm::StringRef name, const Type* type, ExprNode* init, bool constness)
    : DeclNode{ Node::VARIABLE, location, access }, name{ name }, type{ type },
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

bool VariableNode::hasInit() const
{
    return init;
}

ExprNode& VariableNode::getInit() const
{
    return *init;
}

bool VariableNode::isConst() const
{
    return constness;
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

EmptyNode::EmptyNode(Location location)
    : Node{ Node::EMPTY, location }
{
}

void EmptyNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitEmpty(*this);
}

IfNode::IfNode(Location location, ExprNode& condition, Node& thenCase,
    Node* elseCase)
    : Node{ Node::IF, location }, condition{ condition }, thenCase{ thenCase },
    elseCase{ elseCase }
{
}

void IfNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitIf(*this);
}

ExprNode& IfNode::getCondition() const
{
    return condition;
}

Node& IfNode::getThen() const
{
    return thenCase;
}

bool IfNode::hasElse() const
{
    return elseCase;
}

Node& IfNode::getElse() const
{
    return *elseCase;
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

ExprNode& ReturnNode::getValue() const
{
    return *value;
}

ExprNode::ExprNode(Node::Kind kind, Location location)
    : Node{ kind, location }
{
}

bool ExprNode::isExpr() const
{
    return true;
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

UnaryNode::UnaryNode(Location location, UnaryKind op, ExprNode& expr)
    : ExprNode{ Node::UNARY, location }, op{ op }, expr{ expr }
{
}

void UnaryNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitUnary(*this);
}

UnaryKind UnaryNode::getOp() const
{
    return op;
}

const char* UnaryNode::getOpSymbol() const
{
    return unaryKindSymbol(op);
}

ExprNode& UnaryNode::getExpr() const
{
    return expr;
}

BinaryNode::BinaryNode(Location location, BinaryKind op, ExprNode& left,
    ExprNode& right)
    : ExprNode{ Node::BINARY, location }, op{ op }, left{ left }, right{ right }
{
}

void BinaryNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitBinary(*this);
}

BinaryKind BinaryNode::getOp() const
{
    return op;
}

const char* BinaryNode::getOpSymbol() const
{
    return binaryKindSymbol(op);
}

ExprNode& BinaryNode::getLhs() const
{
    return left;
}

ExprNode& BinaryNode::getRhs() const
{
    return right;
}

TernaryNode::TernaryNode(Location location, ExprNode& condition,
    ExprNode& thenCase, ExprNode& elseCase)
    : ExprNode{ Node::TERNARY, location }, condition{ condition },
    thenCase{ thenCase }, elseCase{ elseCase }
{
}

void TernaryNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitTernary(*this);
}

ExprNode& TernaryNode::getCondition() const
{
    return condition;
}

ExprNode& TernaryNode::getThen() const
{
    return thenCase;
}

ExprNode& TernaryNode::getElse() const
{
    return elseCase;
}

CallNode::CallNode(Location location, ExprNode& callee,
    std::vector<ArgNode*> args)
    : ExprNode{ Node::CALL, location }, callee{ callee },
    args{ std::move(args) }
{
}

void CallNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitCall(*this);
}

ExprNode& CallNode::getCallee() const
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

ArgNode& CallNode::getArg(size_t i) const
{
    return *args[i];
}

ArgNode::ArgNode(Location location, llvm::StringRef name, ExprNode& value)
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

ExprNode& ArgNode::getValue() const
{
    return value;
}
