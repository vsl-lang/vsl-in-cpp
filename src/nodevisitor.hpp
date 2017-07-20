#ifndef NODEVISITOR_HPP
#define NODEVISITOR_HPP

class NodeVisitor;

#include "node.hpp"

class NodeVisitor
{
public:
    virtual ~NodeVisitor() = 0;
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
