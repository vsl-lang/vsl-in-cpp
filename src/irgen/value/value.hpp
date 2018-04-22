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
     * @name Relational Operators
     * @{
     */

    bool operator==(const Value& value) const;
    bool operator!=(const Value& value) const;

    /**
     * @}
     * @name Static Factories
     * @{
     */

    static Value getNull();
    static Value getExpr(const Type* vslType, llvm::Value* llvmValue);
    static Value getVar(const Type* vslType, llvm::Value* llvmVar);
    /**
     * Creates a field value.
     *
     * @param base Base object. Must be an expr Value and its type must resolve
     * to a ClassType.
     * @param vslField VSL type of the field.
     * @param llvmField LLVM value of the field.
     * @param destroyBase Whether the base object should be destroyed after use.
     *
     * @returns A field value.
     */
    static Value getField(Value base, const Type* vslField,
        llvm::Value* llvmField, bool destroyBase);
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
    /**
     * Checks whether this value is a variable or field. If true, the LLVM value
     * is guaranteed to be a pointer to the actual value, so a load instruction
     * must be used first to get to it.
     */
    bool isAssignable() const;
    bool isExpr() const;
    /**
     * Gets the VSL type. If this is a field Value, this method returns the type
     * of the field.
     */
    const Type* getVSLType() const;
    /**
     * Gets the LLVM value. If this is a field Value, this method returns the
     * LLVM value of the base object.
     */
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
     * @name Field Getters
     * @{
     */

    bool isField() const;
    const Type* getVSLField() const;
    /**
     * Guaranteed to have a pointer type. A load instruction must be created
     * using this value to get the value of the variable.
     */
    llvm::Value* getLLVMField() const;
    /**
     * Gets the base object. Guaranteed to be an expr Value which has a type
     * that resolve to a ClassType.
     */
    Value getBase() const;
    /**
     * Whether the base object should be destroyed after loading/storing the
     * field.
     */
    bool shouldDestroyBase() const;

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
        /** Field access. */
        FIELD,
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
    /** Data not shared by all Value kinds. */
    union Data
    {
        /** Field Values only. */
        struct
        {
            /** VSL type of base object. */
            const Type* vslType;
            /** LLVM value of base object. */
            llvm::Value* llvmValue;
            /** Whether the base object should be destroyed after use. */
            bool shouldDestroy;
        } base;
    } data;
};

#endif // VALUE_HPP
