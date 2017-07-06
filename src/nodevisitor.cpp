#include "nodevisitor.hpp"

NodeVisitor::~NodeVisitor()
{
}

void NodeVisitor::visit(Node& node)
{
    node.accept(*this);
}
