#ifndef IREMITTER_HPP
#define IREMITTER_HPP

#include "ast/node.hpp"
#include "ast/nodevisitor.hpp"
#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "irgen/scope/funcScope.hpp"
#include "irgen/scope/globalScope.hpp"
#include "irgen/typeConverter/typeConverter.hpp"
#include "irgen/value/value.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <cstdint>

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
     * @param converter VSL to LLVM type converter.
     * @param module The module to emit LLVM IR into.
     */
    IREmitter(VSLContext& vslCtx, Diag& diag, FuncScope& func,
        GlobalScope& global, TypeConverter& converter, llvm::Module& module);
    /**
     * Destroys an IREmitter object.
     */
    virtual ~IREmitter() override = default;
    virtual void visitFunction(FunctionNode& node) override;
    virtual void visitExtFunc(ExtFuncNode& node) override;
    virtual void visitParam(ParamNode& node) override;
    virtual void visitVariable(VariableNode& node) override;
    virtual void visitClass(ClassNode& node) override;
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
     * @name Unary operations
     * @{
     */

    /**
     * Generates a unary negation.
     *
     * @param value The value to operate on. Must be an expr Value.
     */
    void genNeg(Value value);

    /**
     * @}
     * @name Special Binary Operations
     * @{
     */

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
     * @}
     * @name Binary Operations
     * @{
     */

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
     * @}
     * @name Comparison Operations
     * @{
     */

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
     * @}
     * @name Other Helpers
     * @{
     */

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
    /**
     * Checks if this is currently in the global scope.
     *
     * @returns True if in the global scope, false otherwise.
     */
    bool isGlobal() const;
    /**
     * Creates a global variable. This method returns null if the variable
     * already exists.
     *
     * @param access Access specifier.
     * @param vslType VSL type.
     * @param llvmType LLVM type equivalent.
     * @param name Name of the variable.
     *
     * @returns The generated global variable, or null if the name already
     * exists.
     */
    // FIXME: should this return a var value?
    llvm::GlobalVariable* genGlobalVar(Access access, const Type* vslType,
        llvm::Type* llvmType, llvm::StringRef name);
    /**
     * Creates a global variable's constructor function. The function is left
     * only with its entry block generated so that the user can fill in the
     * initialization code and its return instruction.
     *
     * @param var Variable to initialize.
     *
     * @returns The constructor function.
     */
    llvm::Function* genGlobalVarCtor(llvm::GlobalVariable* var);
    /**
     * Generates a global variable's destructor. This function will already be
     * complete so no need to modify it.
     *
     * @param var Variable to initialize.
     * @param type VSL type of the variable.
     */
    void genGlobalVarDtor(llvm::GlobalVariable* var, const Type* type);
    /**
     * Adds a function to be called during program start.
     *
     * @param f The function to be called at runtime. This should take no
     *     parameters and return void.
     */
    void addGlobalCtor(llvm::Function* f);
    /**
     * Adds a function to be called during program end.
     *
     * @param f The function to be called at runtime. This should take no
     *     parameters and return void.
     */
    void addGlobalDtor(llvm::Function* f);
    /**
     * Factors out the common code between addGlobalCtor and addGlobalDtor.
     *
     * @param f The function to be called at runtime. This should take no
     * parameters and return void.
     * @param startOrEnd True if the function should be called at program start,
     * or false for program end.
     */
    void addGlobalCall(llvm::Function* f, bool startOrEnd);
    /**
     * Allocates enough memory to hold an object of the given type.
     *
     * @param type Type to allocate.
     *
     * @returns A call to malloc and a bitcast to the appropriate LLVM type.
     */
    llvm::Value* createMalloc(const Type* type);
    /**
     * Allocates enough memory to hold an object of the given type. Its
     * reference count will also be initialized.
     *
     * @param type Object type to allocate.
     * @param name Named of the type.
     *
     * @returns A call to malloc and a bitcast to the appropriate LLVM type.
     */
    llvm::Value* createMalloc(const ClassType* type, llvm::StringRef name);
    /**
     * Mallocs an object.
     *
     * @param type LLVM type to allocate.
     * @param name Value name.
     *
     * @returns A call to malloc and a bitcast to the appropriate LLVM type.
     */
    llvm::Value* createMalloc(llvm::Type* type, const llvm::Twine& name = "");
    /**
     * Looks up an identifier. Returns null if it doesn't exist. Emits error
     * diagnostics as usual.
     *
     * @param node Node to lookup.
     *
     * @returns The node's Value in the function or global scope.
     */
    Value lookupIdent(IdentNode& node);
    /**
     * Creates a ConstantInt for the first index of a GEP instruction. Note,
     * however, that all other indexes must be of `i32` type.
     *
     * @param ptrType Pointer type to the object being GEP'd.
     * @param i Integer to be used.
     *
     * @returns A constant integer.
     */
    llvm::Constant* createGEPIndex(llvm::Type* ptrType, uint64_t i) const;
    /**
     * Generates the initial function entry block with all the parameters setup.
     *
     * @param node Function to generate.
     * @param funcVal Value that represents the function.
     */
    void setupFuncBody(FunctionNode& node, Value funcVal);
    /**
     * Generates the function body.
     *
     * @param node Function to generate.
     */
    void genFuncBody(FunctionNode& node);
    /**
     * Performs normal function cleanup, like checking for a return at the end
     * and deleting the alloca insert point.
     *
     * @param node Function to generate.
     */
    void cleanupFuncBody(FunctionNode& node);
    /**
     * Creates a call to a function or method with the given arguments. The
     * `result` field will contain the return value. Emits error diagnostics as
     * usual.
     *
     * @param node Contains the function arguments. Callee doesn't matter here.
     * @param funcVal Function or method to call.
     * @param selfArg Optional `self` argument. Applies to methods only.
     */
    void createCall(CallNode& node, Value funcVal,
        Value selfArg = Value::getNull());
    /**
     * Checks if the current scope can access a member of an object.
     *
     * @param objType Type of the object.
     * @param access Access specifier of the member.
     *
     * @returns True if it can be accessed, false otherwise.
     */
    bool canAccessMember(const Type* objType, Access access) const;
    /**
     * Generates a default class destructor. This relies on the FuncResolver
     * having already declared it.
     *
     * @param node Class to create a destructor for.
     */
    void generateDtor(const ClassNode& node);
    /**
     * Attempts to convert a Type to a ClassType. This method returns null if
     * the type can't be resolved to a ClassType.
     *
     * @param type Type to convert.
     *
     * @returns The resulting ClassType, or null if not possible.
     */
    static const ClassType* toClassType(const Type* type);
    /**
     * Loads and copies a Value if it's a variable or field access. Expr copies
     * are elided because they're just temporaries. When copying objects, the
     * reference count is incremented.
     *
     * This is useful in dealing with passing values to/from functions.
     *
     * @param value Value to copy.
     *
     * @returns The resulting Value that was copied and loaded.
     */
    Value copyValue(Value value);
    /**
     * Possibly generates instructions to destroy a value if it's an expression.
     * For objects, this calls its destructor. Assignable values are ignored
     * here because they should've been loaded first.
     *
     * @param value Value to destroy.
     */
    void destroyValue(Value value);
    /**
     * Converts a Value into an expr Value. This creates a load instruction for
     * assignable Values.
     *
     * This method must be called when dealing with expression operands that
     * could be either variables (lvalues) or exprs (rvalues).
     *
     * @param value Value to load.
     *
     * @returns The loaded Value.
     */
    Value loadValue(Value value);
    /**
     * Generates instructions to store an expr value into a var/field value.
     *
     * @param from Value to be stored. Must be an expr value.
     * @param to Value to store into. Must be a var/field value.
     */
    void storeValue(Value from, Value to);
    /**
     * Destroys all variables in the current function scope. Useful when exiting
     * a scope.
     */
    void destroyVars();
    /**
     * Destroys all variables in the entire function scope. Useful in returning
     * from a function.
     */
    void destroyAllVars();

    /** @} */

    /** The VSLContext object to be used. */
    VSLContext& vslCtx;
    /** Diagnostics manager. */
    Diag& diag;
    /** Function scope manager. */
    FuncScope& func;
    /** Global scope manager. */
    GlobalScope& global;
    /** VSL to LLVM type converter. */
    TypeConverter& converter;
    /** LLVM module that holds all the generated code. */
    llvm::Module& module;
    /** Context object that owns most dynamic LLVM data structures. */
    llvm::LLVMContext& llvmCtx;
    /** Used to build the IR. */
    llvm::IRBuilder<> builder;
    /** Points to the instruction where allocas should be inserted before. */
    llvm::Instruction* allocaInsertPoint;
    /** Main global variable constructor function. */
    llvm::Function* vslCtor;
    /** Main global variable destructor function. */
    llvm::Function* vslDtor;
    /**
     * Used as a temporary return value for some visitor methods. Most of the
     * time this is used in the ExprNode visitors so it's best to just set it to
     * `Value::getNull()` otherwise.
     */
    Value result;
    /** Represents the `self` parameter of constructors and methods. */
    Value self;
};

#endif // IREMITTER_HPP
