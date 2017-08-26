#ifndef LLVMGEN_HPP
#define LLVMGEN_HPP

#include "node.hpp"
#include "nodevisitor.hpp"
#include "scopetree.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <iostream>
#include <string>

class LLVMGen : public NodeVisitor
{
public:
    LLVMGen(std::ostream& errors = std::cerr);
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
    std::string getIR() const;

private:
    static llvm::Value* createEntryAlloca(llvm::Function* f,
        llvm::Type* type, const char* name);
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    ScopeTree scopeTree;
    llvm::Value* result;
    std::ostream& errors;
};

#endif // LLVMGEN_HPP
