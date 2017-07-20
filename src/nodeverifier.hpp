#ifndef NODEVERIFIER_HPP
#define NODEVERIFIER_HPP

#include "node.hpp"
#include "nodevisitor.hpp"
#include "scopetree.hpp"
#include <iostream>

class NodeVerifier : public NodeVisitor
{
public:
    NodeVerifier(std::ostream& errors = std::cerr);
    virtual ~NodeVerifier() override;
    virtual void visit(ErrorNode& node) override;
    virtual void visit(EmptyNode& node) override;
    virtual void visit(BlockNode& node) override;
    virtual void visit(ConditionalNode& node) override;
    virtual void visit(AssignmentNode& node) override;
    virtual void visit(FunctionNode& node) override;
    virtual void visit(ReturnNode& node) override;
    virtual void visit(IdentExprNode& node) override;
    virtual void visit(NumberExprNode& node) override;
    virtual void visit(UnaryExprNode& node) override;
    virtual void visit(BinaryExprNode& node) override;
    virtual void visit(CallExprNode& node) override;
    virtual void visit(ArgNode& node) override;

private:
    std::ostream& errors;
    ScopeTree scopeTree;
};

#endif // NODEVERIFIER_HPP
