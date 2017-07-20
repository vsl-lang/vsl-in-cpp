#include "nodeverifier.hpp"

NodeVerifier::NodeVerifier(std::ostream& errors)
    : errors{ errors }
{
}

NodeVerifier::~NodeVerifier()
{
}

void NodeVerifier::visit(ErrorNode& node)
{
    node.type = std::make_unique<SimpleType>(Type::ERROR);
}

void NodeVerifier::visit(EmptyNode& node)
{
    node.type = std::make_unique<SimpleType>(Type::ERROR);
}

void NodeVerifier::visit(BlockNode& node)
{
    node.type = std::make_unique<SimpleType>(Type::ERROR);
    for (auto& statement : node.statements)
    {
        statement->accept(*this);
    }
}

void NodeVerifier::visit(ConditionalNode& node)
{
    node.type = std::make_unique<SimpleType>(Type::ERROR);
    if (scopeTree.isGlobal())
    {
        errors << node.location <<
            ": error: top-level control flow statements are not allowed\n";
    }
    node.condition->accept(*this);
    Type::Kind k = node.condition->type->kind;
    if (k != Type::INT)
    {
        errors << node.condition->location <<
            ": error: cannot convert expression of type " <<
            Type::kindToString(k) << " to type Int\n";
    }
    scopeTree.enter();
    node.thenCase->accept(*this);
    scopeTree.exit();
    scopeTree.enter();
    node.elseCase->accept(*this);
    scopeTree.exit();
}

void NodeVerifier::visit(AssignmentNode& node)
{
    Node& value = *node.value;
    value.accept(*this);
    if (node.type->kind != value.type->kind)
    {
        errors << value.location <<
            ": error: mismatching types when initializing variable " <<
            node.name << '\n';
    }
    if (!scopeTree.set(node.name, node.type.get()))
    {
        errors << node.location << ": error: variable " <<
            node.name << " was already defined\n";
    }
}

void NodeVerifier::visit(FunctionNode& node)
{
    auto& type = static_cast<FunctionType&>(*node.type);
    scopeTree.set(node.name, &type);
    scopeTree.enter(type.returnType.get());
    for (size_t i = 0; i < node.paramNames.size(); ++i)
    {
        scopeTree.set(node.paramNames[i].str, type.params[i].get());
    }
    node.body->accept(*this);
    scopeTree.exit();
}

void NodeVerifier::visit(ReturnNode& node)
{
    node.value->accept(*this);
    node.type = node.value->type->clone();
    if (node.type->kind != scopeTree.getReturnType()->kind)
    {
        errors << node.location <<
            ": error: cannot convert expression of type " <<
            node.type->toString() << " to type " <<
            scopeTree.getReturnType()->toString() << '\n';
    }
}

void NodeVerifier::visit(IdentExprNode& node)
{
    Type* t = scopeTree.get(node.name);
    if (t == nullptr)
    {
        errors << node.location << ": error: unknown variable " <<
            node.name << '\n';
        node.type = std::make_unique<SimpleType>(Type::ERROR);
    }
    else
    {
        node.type = t->clone();
    }
}

void NodeVerifier::visit(NumberExprNode& node)
{
    node.type = std::make_unique<SimpleType>(Type::INT);
}

void NodeVerifier::visit(UnaryExprNode& node)
{
    Node& expr = *node.expr;
    expr.accept(*this);
    Type::Kind k = expr.type->kind;
    switch (k)
    {
    case Type::ERROR:
    case Type::INT:
        node.type = std::make_unique<SimpleType>(k);
        break;
    case Type::VOID:
    default:
        errors << expr.location <<
            ": error: cannot apply unary operator " <<
            Token::kindToString(node.op) << " to type " <<
            expr.type->toString() << '\n';
        node.type = std::make_unique<SimpleType>(Type::ERROR);
    }
}

void NodeVerifier::visit(BinaryExprNode& node)
{
    Node& left = *node.left;
    Node& right = *node.right;
    left.accept(*this);
    right.accept(*this);
    Type::Kind k;
    if (left.type->kind != right.type->kind)
    {
        errors << node.location <<
            ": error: mismatched types for binary operator " <<
            Token::kindToString(node.op) << '\n';
        k = Type::ERROR;
    }
    else
    {
        k = left.type->kind;
    }
    node.type = std::make_unique<SimpleType>(k);
}

void NodeVerifier::visit(CallExprNode& node)
{
    node.callee->accept(*this);
    Type& t = *node.callee->type;
    if (t.kind != Type::FUNCTION)
    {
        errors << node.location << ": error: called object of type " <<
            t.toString() << " is not a function\n";
        node.type = std::make_unique<SimpleType>(Type::ERROR);
        return;
    }
    FunctionType& f = static_cast<FunctionType&>(t);
    if (f.params.size() != node.args.size())
    {
        errors << node.location <<
            ": error: mismatched number of arguments " <<
            node.args.size() << " versus parameters " <<
            f.params.size() << '\n';
    }
    else
    {
        for (size_t i = 0; i < f.params.size(); ++i)
        {
            Node& arg = *node.args[i];
            Type& param = *f.params[i];
            arg.accept(*this);
            if (arg.type->kind != param.kind)
            {
                errors << arg.location << ": error: cannot convert type " <<
                    arg.type->toString() << " to type " << param.toString() <<
                    '\n';
            }
        }
    }
    node.type = f.returnType->clone();
}

void NodeVerifier::visit(ArgNode& node)
{
    node.value->accept(*this);
    node.type = node.value->type->clone();
}
