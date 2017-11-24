#ifndef NODEVISITOR_HPP
#define NODEVISITOR_HPP

class NodeVisitor;

#include "ast/node.hpp"

/**
 * Base class for visiting {@link Node Nodes}, implementing the Visitor Pattern.
 * Subclasses can override the visit methods to perform some operation on a
 * {@link Node} through double dispatch.
 */
class NodeVisitor
{
public:
    virtual ~NodeVisitor() = 0;
    /**
     * Visits a {@link Node}. This is the equivalent of `node.accept(*this)`.
     *
     * @param node The node to visit.
     */
    void visit(Node& node);
    virtual void visitEmpty(EmptyNode& node) = 0;
    virtual void visitBlock(BlockNode& node) = 0;
    virtual void visitIf(IfNode& node) = 0;
    virtual void visitVariable(VariableNode& node) = 0;
    virtual void visitFunction(FunctionNode& node) = 0;
    virtual void visitParam(ParamNode& node) = 0;
    virtual void visitReturn(ReturnNode& node) = 0;
    virtual void visitIdent(IdentNode& node) = 0;
    virtual void visitLiteral(LiteralNode& node) = 0;
    virtual void visitVoid(VoidNode& node) = 0;
    virtual void visitUnary(UnaryNode& node) = 0;
    virtual void visitBinary(BinaryNode& node) = 0;
    virtual void visitCall(CallNode& node) = 0;
    virtual void visitArg(ArgNode& node) = 0;
};

#endif // NODEVISITOR_HPP
