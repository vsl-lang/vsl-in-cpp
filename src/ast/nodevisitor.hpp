#ifndef NODEVISITOR_HPP
#define NODEVISITOR_HPP

class NodeVisitor;

#include "ast/node.hpp"
#include "llvm/ADT/ArrayRef.h"

/**
 * Base class for visiting a Node, implementing the Visitor Pattern. Subclasses
 * can override the visit methods to perform some operation on a Node through
 * double dispatch.
 */
class NodeVisitor
{
public:
    virtual ~NodeVisitor() = 0;
    /**
     * Visits an AST.
     *
     * @param ast The list of global declarations to process.
     */
    virtual void visitAST(llvm::ArrayRef<DeclNode*> ast);
    virtual void visitFunction(FunctionNode& node);
    virtual void visitExtFunc(ExtFuncNode& node);
    virtual void visitParam(ParamNode& node);
    virtual void visitVariable(VariableNode& node);
    virtual void visitClass(ClassNode& node);
    virtual void visitField(FieldNode& node);
    virtual void visitMethod(MethodNode& node);
    virtual void visitCtor(CtorNode& node);
    virtual void visitBlock(BlockNode& node);
    virtual void visitEmpty(EmptyNode& node);
    virtual void visitIf(IfNode& node);
    virtual void visitReturn(ReturnNode& node);
    virtual void visitIdent(IdentNode& node);
    virtual void visitLiteral(LiteralNode& node);
    virtual void visitUnary(UnaryNode& node);
    virtual void visitBinary(BinaryNode& node);
    virtual void visitTernary(TernaryNode& node);
    virtual void visitCall(CallNode& node);
    virtual void visitArg(ArgNode& node);
    virtual void visitFieldAccess(FieldAccessNode& node);
    virtual void visitMethodCall(MethodCallNode& node);
    virtual void visitSelf(SelfNode& node);
};

#endif // NODEVISITOR_HPP
