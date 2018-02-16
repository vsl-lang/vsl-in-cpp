#include "ast/nodePrinter.hpp"

NodePrinter::NodePrinter(llvm::raw_ostream& os)
    : os{ os }, indentLevel{ 0 }
{
}

void NodePrinter::visitStatements(llvm::ArrayRef<Node*> statements)
{
    for (Node* statement : statements)
    {
        // blocks need to indent the inner statements
        if (statement->is(Node::BLOCK))
        {
            ++indentLevel;
        }
        printStatement(*statement);
        if (statement->is(Node::BLOCK))
        {
            --indentLevel;
        }
    }
}

void NodePrinter::visitFunction(FunctionNode& node)
{
    printFuncInterface(node);
    os << '\n';
    ++indentLevel;
    printStatement(*node.getBody());
    --indentLevel;
}

void NodePrinter::visitExtFunc(ExtFuncNode& node)
{
    printFuncInterface(node);
    os << " external(" << node.getAlias() << ");";
}

void NodePrinter::visitParam(ParamNode& node)
{
    os << node.getName() << ": " << *node.getType();
}

void NodePrinter::visitVariable(VariableNode& node)
{
    indent() << accessModPrefix(node.getAccessMod()) <<
        (node.isConst() ? "let " : "var ") << node.getName() << ": " <<
        *node.getType() << " = ";
    node.getInit()->accept(*this);
    os << ';';
}

void NodePrinter::visitBlock(BlockNode& node)
{
    // in most cases this lines up the curly braces with the statement behind
    //  it, which always increments indentLevel with the intention to indent the
    //  statements within, not the actual curly braces
    --indentLevel;
    indent() << "{\n";
    ++indentLevel;
    visitStatements(node.getStatements());
    --indentLevel;
    indent() << '}';
    ++indentLevel;
}

void NodePrinter::visitEmpty(EmptyNode& node)
{
    indent() << ';';
}

void NodePrinter::visitIf(IfNode& node)
{
    indent() << "if (";
    node.getCondition()->accept(*this);
    os << ")\n";
    ++indentLevel;
    printStatement(*node.getThen());
    --indentLevel;
    indent() << "else\n";
    ++indentLevel;
    // FIXME: extra newline here
    printStatement(*node.getElse());
    --indentLevel;
}

void NodePrinter::visitReturn(ReturnNode& node)
{
    indent() << "return";
    if (node.hasValue())
    {
        os << ' ';
        node.getValue()->accept(*this);
    }
    os << ';';
}

void NodePrinter::visitIdent(IdentNode& node)
{
    os << node.getName();
}

void NodePrinter::visitLiteral(LiteralNode& node)
{
    node.getValue().print(os, false);
}

void NodePrinter::visitUnary(UnaryNode& node)
{
    os << node.getOpSymbol() << '(';
    node.getExpr()->accept(*this);
    os << ')';
}

void NodePrinter::visitBinary(BinaryNode& node)
{
    node.getLhs()->accept(*this);
    os << ' ' << node.getOpSymbol() << ' ';
    node.getRhs()->accept(*this);
}

void NodePrinter::visitTernary(TernaryNode& node)
{
    node.getCondition()->accept(*this);
    os << " ? ";
    node.getThen()->accept(*this);
    os << " : ";
    node.getElse()->accept(*this);
}

void NodePrinter::visitCall(CallNode& node)
{
    node.getCallee()->accept(*this);
    os << '(';
    if (node.getNumArgs() > 0)
    {
        node.getArg(0)->accept(*this);
        for (size_t i = 1; i < node.getNumArgs(); ++i)
        {
            os << ", ";
            node.getArg(i)->accept(*this);
        }
    }
    os << ')';
}

void NodePrinter::visitArg(ArgNode& node)
{
    os << node.getName() << ": ";
    node.getValue()->accept(*this);
}

const char* NodePrinter::accessModPrefix(AccessMod access)
{
    switch (access)
    {
    case AccessMod::PUBLIC:
        return "public ";
    case AccessMod::PRIVATE:
        return  "private ";
    default:
        return "";
    }
}

void NodePrinter::printFuncInterface(FuncInterfaceNode& node)
{
    indent() << accessModPrefix(node.getAccessMod()) << "func " <<
        node.getName() << '(';
    if (node.getNumParams() > 0)
    {
        node.getParam(0)->accept(*this);
        for (size_t i = 1; i < node.getNumParams(); ++i)
        {
            os << ", ";
            node.getParam(i)->accept(*this);
        }
    }
    os << ") -> " << *node.getReturnType();
}

void NodePrinter::printStatement(Node& node)
{
    // expression statements don't know if they're being used as a statement or
    //  as, say, a function argument, so we need to manually indent it
    if (node.isExpr())
    {
        indent();
    }
    node.accept(*this);
    // add a semicolon after expression statements
    if (node.isExpr())
    {
        os << ';';
    }
    // the else part of the if statement implicitly adds a newline because of
    //  the statement below
    // this check ensures that there's no trailing newline after an if
    if (node.isNot(Node::IF))
    {
        os << '\n';
    }
}

llvm::raw_ostream& NodePrinter::indent() const
{
    return indent(indentLevel);
}

llvm::raw_ostream& NodePrinter::indent(int level) const
{
    for (int i = 0; i < level; ++i)
    {
        os << "    ";
    }
    return os;
}
