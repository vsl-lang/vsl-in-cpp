#ifndef IREMITTER_HPP
#define IREMITTER_HPP

#include "ast/node.hpp"
#include "ast/nodevisitor.hpp"
#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "irgen/scope/funcScope.hpp"
#include "irgen/scope/globalScope.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

/**
 * Generates LLVM IR by visiting a Node.
 */
class IREmitter : public NodeVisitor
{
public:
    /**
     * Creates an IREmitter object.
     *
     * @param vslCtx The VSLContext object to be used.
     * @param diag Diagnostics manager.
     * @param func Function scope manager.
     * @param global Global scope manager.
     * @param module The module to emit LLVM IR into.
     */
    IREmitter(VSLContext& vslCtx, Diag& diag, FuncScope& func,
        GlobalScope& global, llvm::Module& module);
    /**
     * Destroys an IREmitter object.
     */
    virtual ~IREmitter() override = default;
    virtual void visitEmpty(EmptyNode& node) override;
    virtual void visitBlock(BlockNode& node) override;
    virtual void visitIf(IfNode& node) override;
    virtual void visitVariable(VariableNode& node) override;
    virtual void visitFunction(FunctionNode& node) override;
    virtual void visitExtFunc(ExtFuncNode& node) override;
    virtual void visitParam(ParamNode& node) override;
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
     * Generates a variable assignment.
     *
     * @param node The expression to generate code for.
     */
    void genAssign(BinaryNode& node);
    /**
     * Generates a short-circuiting boolean operation, i.e., and/or. Don't call
     * this method if `node.getOp()` is anything but TokenKind::AND or
     * TokenKind::OR.
     *
     * @param node The expression to generate code for.
     */
    void genShortCircuit(BinaryNode& node);
    /**
     * Generates a unary negation.
     *
     * @param type The VSL type of the operand.
     * @param value The LLVM value to operate on.
     */
    void genNeg(const Type* type, llvm::Value* value);
    /**
     * Generates a binary add instruction.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genAdd(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates a binary subtract instruction.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genSub(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates a binary multiply instruction.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genMul(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates a binary divide instruction.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genDiv(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates a binary remainder instruction.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genMod(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates an equals comparison.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genEQ(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates a not equals comparison.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genNE(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates a greater than comparison.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genGT(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates a greater than or equal comparison.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genGE(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates a less than comparison.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genLT(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Generates a less than or equal comparison.
     *
     * @param type The VSL type of the operand.
     * @param lhs Left operand.
     * @param rhs Right operand.
     */
    void genLE(const Type* type, llvm::Value* lhs, llvm::Value* rhs);
    /**
     * Creates an `alloca` instruction in the entry block of the current
     * function. This is useful for stuff like variables and function parameters
     * to make the `alloca`ing process a little bit easier.
     *
     * @param type LLVM type representing the memory block to allocate.
     * @param name Name of the memory block.
     *
     * @returns An alloca instruction.
     */
    llvm::AllocaInst* createEntryAlloca(llvm::Type* type,
        const llvm::Twine& name = "");
    /**
     * Creates a branch instruction to the target block. If there is no
     * insertion point, or the current block is already terminated, then this
     * method does nothing.
     *
     * @param target The block to branch to.
     *
     * @returns The branch instruction that was created, or nullptr otherwise.
     */
    llvm::BranchInst* branchTo(llvm::BasicBlock* target);
    /** The VSLContext object to be used. */
    VSLContext& vslCtx;
    /** Diagnostics manager. */
    Diag& diag;
    /** Function scope manager. */
    FuncScope& func;
    /** Global scope manager. */
    GlobalScope& global;
    /** Context object that owns most dynamic LLVM data structures. */
    llvm::LLVMContext& llvmCtx;
    /** Used to build the IR. */
    llvm::IRBuilder<> builder;
    /** Points to the instruction where allocas should be inserted before. */
    llvm::Instruction* allocaInsertPoint;
    /**
     * Used as a temporary return value for some visitor methods. Most of the
     * time this is used in the ExprNode visitors so it's best to just set it to
     * `nullptr` otherwise.
     */
    llvm::Value* result;
};

#endif // IREMITTER_HPP
