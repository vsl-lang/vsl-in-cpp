#ifndef LLVMGEN_HPP
#define LLVMGEN_HPP

#include "ast/node.hpp"
#include "ast/nodevisitor.hpp"
#include "irgen/scopetree.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <string>

/**
 * Generates LLVM IR by visiting a {@link Node}.
 */
class LLVMGen : public NodeVisitor
{
public:
    /**
     * Creates an LLVMGen object.
     *
     * @param module The module to emit LLVM IR into.
     * @param errors The stream to print errors to.
     */
    LLVMGen(llvm::Module& module, std::ostream& errors = std::cerr);
    /**
     * Destroys an LLVMGen object.
     */
    virtual ~LLVMGen() override = default;
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
    /**
     * Dumps the generated LLVM IR in string form.
     *
     * @returns The string representation of the IR.
     */
    std::string getIR() const;
    /**
     * Checks if an error has been encountered yet.
     *
     * @returns True if an error was encountered, false otherwise.
     */
    bool hasError() const;

private:
    /**
     * Creates an `alloca` instruction in the entry block of the current
     * function. This is useful for stuff like variables and function parameters
     * to make the `alloca`ing process a little bit easier.
     *
     * @param f The function to insert the `alloca` into.
     * @param type An LLVM type representing the memory block to allocate.
     * @param name The name of the memory block.
     *
     * @returns An LLVM AllocaInst.
     */
    static llvm::Value* createEntryAlloca(llvm::Function* f, llvm::Type* type,
        const char* name);
    /**
     * The module to emit LLVM IR into.
     */
    llvm::Module& module;
    /**
     * Context object that owns most dynamic LLVM data structures.
     */
    llvm::LLVMContext& context;
    /**
     * Used to build the IR.
     */
    llvm::IRBuilder<> builder;
    /**
     * Manages the current scope.
     */
    ScopeTree scopeTree;
    /**
     * Used as a temporary return value for some visitor methods. Most of the
     * time this is used in the {@link ExprNode} visitors so it's best to just
     * set it to `nullptr` otherwise.
     */
    llvm::Value* result;
    /**
     * The stream to print errors to.
     */
    std::ostream& errors;
    /**
     * True if an error was encountered, otherwise false.
     */
    bool errored;
};

#endif // LLVMGEN_HPP
