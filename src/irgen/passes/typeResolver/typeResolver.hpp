#ifndef TYPERESOLVER_HPP
#define TYPERESOLVER_HPP

#include "ast/node.hpp"
#include "ast/nodevisitor.hpp"
#include "ast/vslContext.hpp"
#include "irgen/typeConverter/typeConverter.hpp"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

/**
 * Gathers information on type declarations to resolve {@link UnresolvedType
 * UnresolvedTypes}.
 */
class TypeResolver : public NodeVisitor
{
public:
    /**
     * Creates a TypeResolver.
     *
     * @param vslCtx Context object.
     * @param converter VSL to LLVM type converter.
     * @param module Used for declaring LLVM types.
     */
    TypeResolver(VSLContext& vslCtx, TypeConverter& converter,
        llvm::Module& module);
    virtual ~TypeResolver() override = default;
    virtual void visitAST(llvm::ArrayRef<DeclNode*> ast) override;
    virtual void visitClass(ClassNode& node) override;

private:
    /**
     * The sequence of passes to be run on the AST.
     */
    enum Pass
    {
        /** Corresponds to gatherInfo. */
        PASS_1,
        /** Corresponds to resolve. */
        PASS_2
    };
    /**
     * Gathers type info about a class. This declares the LLVM struct type in
     * the TypeConverter.
     *
     * @param node Class to gather info about.
     */
    void gatherInfo(ClassNode& node);
    /**
     * Resolves the field types of the given class. This defines the body of the
     * LLVM struct type.
     *
     * @param node Class to resolve.
     */
    void resolve(ClassNode& node);
    /** Context object. */
    VSLContext& vslCtx;
    /** VSL to LLVM type converter. */
    TypeConverter& converter;
    /** Used for declaring LLVM types. */
    llvm::Module& module;
    /** LLVM context object. */
    llvm::LLVMContext& llvmCtx;
    /** Current AST pass to do. */
    Pass pass;
};

#endif // TYPERESOLVER_HPP
