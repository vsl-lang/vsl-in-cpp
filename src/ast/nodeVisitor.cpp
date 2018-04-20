#include "ast/nodeVisitor.hpp"

NodeVisitor::~NodeVisitor() = default;

void NodeVisitor::visitAST(llvm::ArrayRef<DeclNode*> ast)
{
    for (DeclNode* decl : ast)
    {
        decl->accept(*this);
    }
}

void NodeVisitor::visitFunction(FunctionNode& node)
{
}

void NodeVisitor::visitExtFunc(ExtFuncNode& node)
{
}

void NodeVisitor::visitParam(ParamNode& node)
{
}

void NodeVisitor::visitTypealias(TypealiasNode& node)
{
}

void NodeVisitor::visitVariable(VariableNode& node)
{
}

void NodeVisitor::visitClass(ClassNode& node)
{
}

void NodeVisitor::visitField(FieldNode& node)
{
}

void NodeVisitor::visitMethod(MethodNode& node)
{
}

void NodeVisitor::visitCtor(CtorNode& node)
{
}

void NodeVisitor::visitBlock(BlockNode& node)
{
}

void NodeVisitor::visitEmpty(EmptyNode& node)
{
}

void NodeVisitor::visitIf(IfNode& node)
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

void NodeVisitor::visitFieldAccess(FieldAccessNode& node)
{
}

void NodeVisitor::visitMethodCall(MethodCallNode& node)
{
}

void NodeVisitor::visitSelf(SelfNode& node)
{
}
