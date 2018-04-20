#ifndef NODEPRINTER_HPP
#define NODEPRINTER_HPP

#include "ast/node.hpp"
#include "ast/nodeVisitor.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/raw_ostream.h"

/**
 * Pretty-prints the AST.
 */
class NodePrinter : public NodeVisitor
{
public:
    /**
     * Creates a NodePrinter.
     *
     * @param os Stream to print to.
     */
    NodePrinter(llvm::raw_ostream& os);
    virtual ~NodePrinter() override = default;
    virtual void visitAST(llvm::ArrayRef<DeclNode*> ast) override;
    virtual void visitFunction(FunctionNode& node) override;
    virtual void visitExtFunc(ExtFuncNode& node) override;
    virtual void visitParam(ParamNode& node) override;
    virtual void visitTypealias(TypealiasNode& node) override;
    virtual void visitVariable(VariableNode& node) override;
    virtual void visitClass(ClassNode& node) override;
    virtual void visitField(FieldNode& node) override;
    virtual void visitMethod(MethodNode& node) override;
    virtual void visitCtor(CtorNode& node) override;
    virtual void visitBlock(BlockNode& node) override;
    virtual void visitEmpty(EmptyNode& node) override;
    virtual void visitIf(IfNode& node) override;
    virtual void visitReturn(ReturnNode& node) override;
    virtual void visitIdent(IdentNode& node) override;
    virtual void visitLiteral(LiteralNode& node) override;
    virtual void visitUnary(UnaryNode& node) override;
    virtual void visitBinary(BinaryNode& node) override;
    virtual void visitTernary(TernaryNode& node) override;
    virtual void visitCall(CallNode& node) override;
    virtual void visitArg(ArgNode& node) override;
    virtual void visitFieldAccess(FieldAccessNode& node) override;
    virtual void visitMethodCall(MethodCallNode& node) override;
    virtual void visitSelf(SelfNode& node) override;

private:
    /**
     * Gets an access specifier in string form with a space at the end. NONE
     * will just return an empty string.
     *
     * @param access Access specifier.
     */
    static const char* accessPrefix(Access access);
    /**
     * Prints a function's interface. This excludes the body or the external
     * alias declaration.
     */
    void printFuncInterface(FuncInterfaceNode& node);
    /**
     * Prints a comma-separated list of Nodes wrapped in parentheses.
     *
     * @tparam NodeT Node-derived type to print.
     *
     * @param nodes List of nodes to print.
     */
    template<typename NodeT>
    void printNodeList(llvm::ArrayRef<NodeT*> nodes);
    /**
     * Prints a left brace, used to open a block, with correct indentation and a
     * newline at the end.
     */
    void openBlock();
    /**
     * Prints a right brace, used to close a block, with correct indentation.
     */
    void closeBlock();
    /**
     * Prints a single statement, with special handling for blocks and
     * expression statements.
     *
     * @param node Node to print.
     */
    void printStatement(Node& node);
    /**
     * Prints an indent.
     *
     * @returns A reference to the stored raw_ostream field.
     */
    llvm::raw_ostream& indent() const;
    /**
     * Prints an indent.
     *
     * @param level Indentation level.
     *
     * @returns A reference to the stored raw_ostream field.
     */
    llvm::raw_ostream& indent(int level) const;
    /** Stream to print to. */
    llvm::raw_ostream& os;
    /** Current indentation level. */
    int indentLevel;
};

#endif // NODEPRINTER_HPP
