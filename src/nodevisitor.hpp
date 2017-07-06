#ifndef NODEVISITOR_HPP
#define NODEVISITOR_HPP

class NodeVisitor;

#include "node.hpp"

class NodeVisitor
{
public:
    virtual ~NodeVisitor() = 0;
    void visit(Node& node);
    virtual void visit(ErrorNode& errorNode) = 0;
    virtual void visit(EmptyNode& emptyNode) = 0;
    virtual void visit(BlockNode& blockNode) = 0;
    virtual void visit(ConditionalNode& conditionalNode) = 0;
    virtual void visit(AssignmentNode& assignmentNode) = 0;
    virtual void visit(FunctionNode& functionNode) = 0;
    virtual void visit(ReturnNode& returnNode) = 0;
    virtual void visit(ParamNode& paramNode) = 0;
    virtual void visit(TypeNode& typeNode) = 0;
    virtual void visit(IdentExprNode& identExprNode) = 0;
    virtual void visit(NumberExprNode& numberExprNode) = 0;
    virtual void visit(UnaryExprNode& unaryExprNode) = 0;
    virtual void visit(BinaryExprNode& binaryExprNode) = 0;
    virtual void visit(CallExprNode& callExprNode) = 0;
    virtual void visit(ArgNode& argNode) = 0;
};

#endif // NODEVISITOR_HPP
