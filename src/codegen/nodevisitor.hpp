#ifndef NODEVISITOR_HPP
#define NODEVISITOR_HPP

class NodeVisitor;

#include "parser/node.hpp"

/**
 * Base class for visiting {@link Node Nodes}, implementing the Visitor Pattern.
 * Subclasses can override the visit methods to perform some operation on a
 * {@link Node} through double dispatch.
 */
class NodeVisitor
{
public:
    /**
     * Destroys a NodeVisitor.
     */
    virtual ~NodeVisitor() = 0;
    /**
     * Visits a {@link Node}. Because this method is hidden by its overloaded
     * counterparts, subclasses must add a `using NodeVisitor::visit;` statement
     * in order to use it. Usually, it's fine to just use {@link Node::accept}
     * instead.
     *
     * @param node The node to visit.
     */
    void visit(Node& node);
    virtual void visit(ErrorNode& node) = 0;
    virtual void visit(EmptyNode& node) = 0;
    virtual void visit(BlockNode& node) = 0;
    virtual void visit(ConditionalNode& node) = 0;
    virtual void visit(AssignmentNode& node) = 0;
    virtual void visit(FunctionNode& node) = 0;
    virtual void visit(ReturnNode& node) = 0;
    virtual void visit(IdentExprNode& node) = 0;
    virtual void visit(NumberExprNode& node) = 0;
    virtual void visit(UnaryExprNode& node) = 0;
    virtual void visit(BinaryExprNode& node) = 0;
    virtual void visit(CallExprNode& node) = 0;
    virtual void visit(ArgNode& node) = 0;
};

#endif // NODEVISITOR_HPP
