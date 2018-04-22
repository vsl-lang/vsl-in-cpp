#include "ast/node.hpp"

Access mergeAccess(Access parent, Access child)
{
    if (parent == Access::PUBLIC)
    {
        return child;
    }
    return parent;
}

Access keywordToAccess(TokenKind kind)
{
    switch (kind)
    {
    case TokenKind::KW_PUBLIC:
        return Access::PUBLIC;
    case TokenKind::KW_PRIVATE:
        return Access::PRIVATE;
    default:
        return Access::NONE;
    }
}

llvm::GlobalValue::LinkageTypes accessToLinkage(Access access)
{
    if (access == Access::PUBLIC)
    {
        return llvm::GlobalValue::ExternalLinkage;
    }
    return llvm::GlobalValue::InternalLinkage;
}

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

DeclNode::DeclNode(Node::Kind kind, Location location, Access access)
    : Node{ kind, location }, access{ access }
{
}

Access DeclNode::getAccess() const
{
    return access;
}

FuncInterfaceNode::FuncInterfaceNode(Node::Kind kind, Location location,
    Access access, llvm::StringRef name, std::vector<ParamNode*> params,
    const Type* returnType)
    : DeclNode{ kind, location, access }, name{ name },
    params{ std::move(params) }, returnType{ returnType }
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

FunctionNode::FunctionNode(Location location, Access access,
    llvm::StringRef name, std::vector<ParamNode*> params,
    const Type* returnType, BlockNode& body)
    : FunctionNode{ Node::FUNCTION, location, access, name, std::move(params),
        returnType, body }
{
}

void FunctionNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitFunction(*this);
}

BlockNode& FunctionNode::getBody() const
{
    return body;
}

bool FunctionNode::isAlreadyDefined() const
{
    return alreadyDefined;
}

void FunctionNode::setAlreadyDefined(bool alreadyDefined)
{
    this->alreadyDefined = alreadyDefined;
}

FunctionNode::FunctionNode(Node::Kind kind, Location location, Access access,
    llvm::StringRef name, std::vector<ParamNode*> params,
    const Type* returnType, BlockNode& body)
    : FuncInterfaceNode{ kind, location, access, name, std::move(params),
        returnType }, body{ body }, alreadyDefined{ false }
{
}

ExtFuncNode::ExtFuncNode(Location location, Access access, llvm::StringRef name,
    std::vector<ParamNode*> params, const Type* returnType,
    llvm::StringRef alias)
    : FuncInterfaceNode{ Node::EXTFUNC, location, access, name,
        std::move(params), returnType }, alias{ alias }
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

VariableNode::VariableNode(Location location, Access access,
    llvm::StringRef name, const Type* type, ExprNode* init, bool constness)
    : VariableNode{ Node::VARIABLE, location, access, name, type, init,
        constness }
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

bool VariableNode::hasType() const
{
    return type;
}

const Type* VariableNode::getType() const
{
    return type;
}

void VariableNode::setType(const Type* type)
{
    this->type = type;
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

VariableNode::VariableNode(Node::Kind kind, Location location, Access access,
    llvm::StringRef name, const Type* type, ExprNode* init, bool constness)
    : DeclNode{ kind, location, access }, name{ name }, type{ type },
    init{ init }, constness{ constness }
{
}


ClassNode::Member::Member(ClassNode& parent)
    : parent { parent }
{
}

ClassNode& ClassNode::Member::getParent() const
{
    return parent;
}

ClassNode::ClassNode(Location location, Access access, llvm::StringRef name,
    ClassType* type)
    : DeclNode{ Node::CLASS, location, access }, name{ name }, type{ type },
    ctor{ nullptr }
{
}

void ClassNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitClass(*this);
}

llvm::StringRef ClassNode::getName() const
{
    return name;
}

const ClassType* ClassNode::getType() const
{
    return type;
}

llvm::ArrayRef<FieldNode*> ClassNode::getFields() const
{
    return fields;
}

size_t ClassNode::getNumFields() const
{
    return fields.size();
}

FieldNode& ClassNode::getField(size_t i) const
{
    return *fields[i];
}

bool ClassNode::hasCtor() const
{
    return ctor;
}

CtorNode& ClassNode::getCtor() const
{
    return *ctor;
}

llvm::ArrayRef<MethodNode*> ClassNode::getMethods() const
{
    return methods;
}

bool ClassNode::addField(FieldNode& field)
{
    // attempt to add a field to the ClassType
    if (type->setField(field.getName(), field.getType(), fields.size(),
            field.getAccess()))
    {
        // field already exists
        return true;
    }
    // everything's fine, add to field node list
    fields.push_back(&field);
    return false;
}

void ClassNode::setCtor(CtorNode& ctor)
{
    this->ctor = &ctor;
}

void ClassNode::addMethod(MethodNode& method)
{
    methods.push_back(&method);
}

FieldNode::FieldNode(Location location, Access access, llvm::StringRef name,
    const Type* type, ExprNode* init, bool constness, ClassNode& parent)
    : VariableNode{ Node::FIELD, location, access, name, type, init,
        constness },
    ClassNode::Member{ parent }
{
}

void FieldNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitField(*this);
}

MethodNode::MethodNode(Location location, Access access, llvm::StringRef name,
    std::vector<ParamNode*> params, const Type* returnType, BlockNode& body,
    ClassNode& parent)
    : FunctionNode{ Node::METHOD, location, access, name, std::move(params),
        returnType, body }, ClassNode::Member{ parent }
{
}

void MethodNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitMethod(*this);
}

CtorNode::CtorNode(Location location, Access access,
    std::vector<ParamNode*> params, BlockNode& body, ClassNode& parent)
    : FunctionNode{ Node::CTOR, location, access, parent.getName(),
        std::move(params), parent.getType(), body }, ClassNode::Member{ parent }
{
}

void CtorNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitCtor(*this);
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
    : CallNode{ Node::CALL, location, callee, std::move(args) }
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

CallNode::CallNode(Node::Kind kind, Location location, ExprNode& callee,
    std::vector<ArgNode*> args)
    : ExprNode{ kind, location }, callee{ callee }, args{ std::move(args) }
{
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

FieldAccessNode::FieldAccessNode(Location location, ExprNode& object,
    llvm::StringRef field)
    : ExprNode{ Node::FIELD_ACCESS, location }, object{ object }, field{ field }
{
}

void FieldAccessNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitFieldAccess(*this);
}

ExprNode& FieldAccessNode::getObject() const
{
    return object;
}

llvm::StringRef FieldAccessNode::getField() const
{
    return field;
}

MethodCallNode::MethodCallNode(Location location, ExprNode& callee,
    llvm::StringRef method, std::vector<ArgNode*> args)
    : CallNode{ Node::METHOD_CALL, location, callee, std::move(args) },
    method{ method }
{
}

void MethodCallNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitMethodCall(*this);
}

llvm::StringRef MethodCallNode::getMethod() const
{
    return method;
}

SelfNode::SelfNode(Location location)
    : ExprNode{ Node::SELF, location }
{
}

void SelfNode::accept(NodeVisitor& nodeVisitor)
{
    nodeVisitor.visitSelf(*this);
}
