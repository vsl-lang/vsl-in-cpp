#include "ast/nodevisitor.hpp"

NodeVisitor::~NodeVisitor()
{
}

void NodeVisitor::visitStatements(llvm::MutableArrayRef<Node*> statements)
{
    for (Node* statement : statements)
    {
        statement->accept(*this);
    }
}

void NodeVisitor::visitEmpty(EmptyNode& node)
{
}

void NodeVisitor::visitBlock(BlockNode& node)
{
}

void NodeVisitor::visitIf(IfNode& node)
{
}

void NodeVisitor::visitVariable(VariableNode& node)
{
}

void NodeVisitor::visitFunction(FunctionNode& node)
{
}

void NodeVisitor::visitParam(ParamNode& node)
{
}

void NodeVisitor::visitReturn(ReturnNode& node)
{
}

void NodeVisitor::visitIdent(IdentNode& node)
{
}

void NodeVisitor::visitLiteral(LiteralNode& node)
{
}

void NodeVisitor::visitUnary(UnaryNode& node)
{
}

void NodeVisitor::visitBinary(BinaryNode& node)
{
}

void NodeVisitor::visitTernary(TernaryNode& node)
{
}

void NodeVisitor::visitCall(CallNode& node)
{
}

void NodeVisitor::visitArg(ArgNode& node)
{
}
