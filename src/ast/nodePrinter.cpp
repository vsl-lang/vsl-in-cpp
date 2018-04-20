#include "ast/nodePrinter.hpp"

NodePrinter::NodePrinter(llvm::raw_ostream& os)
    : os{ os }, indentLevel{ 0 }
{
}

void NodePrinter::visitAST(llvm::ArrayRef<DeclNode*> ast)
{
    for (DeclNode* decl : ast)
    {
        decl->accept(*this);
        os << '\n';
    }
}

void NodePrinter::visitFunction(FunctionNode& node)
{
    printFuncInterface(node);
    os << '\n';
    ++indentLevel;
    visitBlock(node.getBody());
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

void NodePrinter::visitTypealias(TypealiasNode& node)
{
    os << "typealias " << node.getName() << " = " << *node.getType() << ';';
}

void NodePrinter::visitVariable(VariableNode& node)
{
    indent() << accessPrefix(node.getAccess()) <<
        (node.isConst() ? "let " : "var ") << node.getName();
    if (node.hasType())
    {
        os << ": " << *node.getType();
    }
    if (node.hasInit())
    {
        os << " = ";
        node.getInit().accept(*this);
    }
    os << ';';
}

void NodePrinter::visitClass(ClassNode& node)
{
    indent() << accessPrefix(node.getAccess()) << "class " << node.getName() <<
        '\n';
    ++indentLevel;
    openBlock();
    // print fields
    for (FieldNode* field : node.getFields())
    {
        field->accept(*this);
        os << '\n';
    }
    // print ctor
    if (node.hasCtor())
    {
        node.getCtor().accept(*this);
        os << '\n';
    }
    // print methods
    for (MethodNode* method : node.getMethods())
    {
        method->accept(*this);
        os << '\n';
    }
    closeBlock();
    --indentLevel;
}

void NodePrinter::visitField(FieldNode& node)
{
    // handled like a normal var
    visitVariable(node);
}

void NodePrinter::visitMethod(MethodNode& node)
{
    visitFunction(node);
}

void NodePrinter::visitCtor(CtorNode& node)
{
    indent() << accessPrefix(node.getAccess()) << "init";
    printNodeList(node.getParams());
    os << '\n';
    ++indentLevel;
    visitBlock(node.getBody());
    --indentLevel;
}

void NodePrinter::visitBlock(BlockNode& node)
{
    openBlock();
    // print each statement inside the block
    for (Node* statement : node.getStatements())
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
    closeBlock();
}

void NodePrinter::visitEmpty(EmptyNode& node)
{
    indent() << ';';
}

void NodePrinter::visitIf(IfNode& node)
{
    indent() << "if (";
    node.getCondition().accept(*this);
    os << ")\n";
    ++indentLevel;
    printStatement(node.getThen());
    --indentLevel;
    if (node.hasElse())
    {
        indent() << "else\n";
        ++indentLevel;
        // FIXME: printing any statement adds a newline after, so technically
        //  two newlines are added after printing the else case
        printStatement(node.getElse());
        --indentLevel;
    }
}

void NodePrinter::visitReturn(ReturnNode& node)
{
    indent() << "return";
    if (node.hasValue())
    {
        os << ' ';
        node.getValue().accept(*this);
    }
    os << ';';
}

void NodePrinter::visitIdent(IdentNode& node)
{
    os << node.getName();
}

void NodePrinter::visitLiteral(LiteralNode& node)
{
    if (node.getValue().getBitWidth() == 1)
    {
        // print as a bool
        os << (node.getValue().getBoolValue() ? "true" : "false");
    }
    else
    {
        // print as a regular int
        node.getValue().print(os, false);
    }
}

void NodePrinter::visitUnary(UnaryNode& node)
{
    os << node.getOpSymbol() << '(';
    node.getExpr().accept(*this);
    os << ')';
}

void NodePrinter::visitBinary(BinaryNode& node)
{
    node.getLhs().accept(*this);
    os << ' ' << node.getOpSymbol() << ' ';
    node.getRhs().accept(*this);
}

void NodePrinter::visitTernary(TernaryNode& node)
{
    node.getCondition().accept(*this);
    os << " ? ";
    node.getThen().accept(*this);
    os << " : ";
    node.getElse().accept(*this);
}

void NodePrinter::visitCall(CallNode& node)
{
    node.getCallee().accept(*this);
    printNodeList(node.getArgs());
}

void NodePrinter::visitArg(ArgNode& node)
{
    os << node.getName() << ": ";
    node.getValue().accept(*this);
}

void NodePrinter::visitFieldAccess(FieldAccessNode& node)
{
    node.getObject().accept(*this);
    os << '.' << node.getField();
}

void NodePrinter::visitMethodCall(MethodCallNode& node)
{
    node.getCallee().accept(*this);
    os << '.' << node.getMethod();
    printNodeList(node.getArgs());
}

void NodePrinter::visitSelf(SelfNode& node)
{
    os << "self";
}

const char* NodePrinter::accessPrefix(Access access)
{
    switch (access)
    {
    case Access::PUBLIC:
        return "public ";
    case Access::PRIVATE:
        return  "private ";
    default:
        return "";
    }
}

void NodePrinter::printFuncInterface(FuncInterfaceNode& node)
{
    indent() << accessPrefix(node.getAccess()) << "func " <<
        node.getName();
    printNodeList(node.getParams());
    os << " -> " << *node.getReturnType();
}

template<typename NodeT>
void NodePrinter::printNodeList(llvm::ArrayRef<NodeT*> nodes)
{
    os << '(';
    if (!nodes.empty())
    {
        nodes[0]->accept(*this);
        for (size_t i = 1; i < nodes.size(); ++i)
        {
            os << ", ";
            nodes[i]->accept(*this);
        }
    }
    os << ')';
}

void NodePrinter::openBlock()
{
    // usually we want to indent the statements within, not the actual braces
    --indentLevel;
    indent() << "{\n";
    ++indentLevel;
}

void NodePrinter::closeBlock()
{
    --indentLevel;
    indent() << '}';
    ++indentLevel;
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
