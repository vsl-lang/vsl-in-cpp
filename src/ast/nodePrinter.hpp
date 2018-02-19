#ifndef NODEPRINTER_HPP
#define NODEPRINTER_HPP

#include "ast/node.hpp"
#include "ast/nodevisitor.hpp"
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
    virtual void visitVariable(VariableNode& node) override;
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

private:
    /**
     * Gets an AccessMod in string form with a space at the end (if applicable).
     * No access type just returns an empty string.
     *
     * @param access Access modifier.
     */
    static const char* accessModPrefix(AccessMod access);
    /**
     * Prints a function's interface. This excludes the body or the external
     * alias declaration.
     */
    void printFuncInterface(FuncInterfaceNode& node);
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
