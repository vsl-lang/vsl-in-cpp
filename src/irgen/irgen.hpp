#ifndef LLVMGEN_HPP
#define LLVMGEN_HPP

#include "ast/node.hpp"
#include "ast/nodevisitor.hpp"
#include "irgen/scopetree.hpp"
#include "irgen/vslContext.hpp"
#include "llvm/ADT/StringRef.h"
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
class IRGen : public NodeVisitor
{
public:
    /**
     * Creates an IRGen object.
     *
     * @param vslContext The VSLContext object to be used.
     * @param module The module to emit LLVM IR into.
     * @param errors The stream to print errors to.
     */
    IRGen(VSLContext& vslContext, llvm::Module& module,
        std::ostream& errors = std::cerr);
    /**
     * Destroys an IRGen object.
     */
    virtual ~IRGen() override = default;
    virtual void visitEmpty(EmptyNode& node) override;
    virtual void visitBlock(BlockNode& node) override;
    virtual void visitIf(IfNode& node) override;
    virtual void visitVariable(VariableNode& node) override;
    virtual void visitFunction(FunctionNode& node) override;
    virtual void visitParam(ParamNode& node) override;
    virtual void visitReturn(ReturnNode& node) override;
    virtual void visitIdent(IdentNode& node) override;
    virtual void visitLiteral(LiteralNode& node) override;
    virtual void visitUnary(UnaryNode& node) override;
    virtual void visitBinary(BinaryNode& node) override;
    virtual void visitCall(CallNode& node) override;
    virtual void visitArg(ArgNode& node) override;
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
        llvm::StringRef name);
    /* The VSLContext object to be used. */
    VSLContext& vslContext;
    /** The module to emit LLVM IR into. */
    llvm::Module& module;
    /** Context object that owns most dynamic LLVM data structures. */
    llvm::LLVMContext& context;
    /** Used to build the IR. */
    llvm::IRBuilder<> builder;
    /** Manages the current scope. */
    ScopeTree scopeTree;
    /**
     * Used as a temporary return value for some visitor methods. Most of the
     * time this is used in the {@link ExprNode} visitors so it's best to just
     * set it to `nullptr` otherwise.
     */
    llvm::Value* result;
    /** The stream to print errors to. */
    std::ostream& errors;
    /** True if an error was encountered, otherwise false. */
    bool errored;
};

#endif // LLVMGEN_HPP
