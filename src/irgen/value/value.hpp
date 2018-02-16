#ifndef VALUE_HPP
#define VALUE_HPP

#include "ast/type.hpp"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"

/**
 * Wraps a VSL type and an LLVM value. This can be used with expressions and
 * other stuff that require bridging the gap between VSL and LLVM.
 */
class Value
{
public:
    Value();

    /**
     * @name Static Factories
     * @{
     */

    static Value getNull();
    static Value getExpr(const Type* vslType, llvm::Value* llvmValue);
    static Value getVar(const Type* vslType, llvm::Value* llvmVar);
    static Value getFunc(const FunctionType* funcType,
        llvm::Function* llvmFunc);

    /**
     * @}
     * @name Generic Getters
     * @{
     */

    /**
     * Checks whether the Value is valid.
     *
     * @returns True if valid, false otherwise.
     */
    bool isValid() const;
    /**
     * Checks whether the Value is not valid.
     *
     * @returns True if invalid, false otherwise.
     */
    bool operator!() const;
    /**
     * Checks whether the Value is valid.
     *
     * @returns True if valid, false otherwise.
     */
    operator bool() const;
    bool isExpr() const;
    const Type* getVSLType() const;
    llvm::Value* getLLVMValue() const;

    /**
     * @}
     * @name Variable Getters
     * @{
     */

    bool isVar() const;
    const Type* getVSLVar() const;
    /**
     * Guaranteed to have a pointer type. A load instruction must be created
     * using this value to get the value of the variable.
     */
    llvm::Value* getLLVMVar() const;

    /**
     * @}
     * @name Function Getters
     * @{
     */

    bool isFunc() const;
    const FunctionType* getVSLFunc() const;
    llvm::Function* getLLVMFunc() const;

    /** @} */

private:
    /**
     * The kind of Value this is.
     */
    enum class Kind
    {
        /** Invalid. */
        INVALID,
        /** Expression. */
        EXPR,
        /** Local variable. */
        VAR,
        /** Function. */
        FUNC
    };

    /**
     * Private constructor.
     *
     * @param kind How vslType/llvmValue should be treated.
     * @param vslType VSL type.
     * @param llvmValue LLVM value.
     */
    Value(Kind kind, const Type* vslType, llvm::Value* llvmValue);

    /** Determines how to treat vslType/llvmValue. */
    Kind kind;
    /** VSL type. */
    const Type* vslType;
    /** LLVM value. */
    llvm::Value* llvmValue;
};

#endif // VALUE_HPP
