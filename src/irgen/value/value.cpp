#include "irgen/value/value.hpp"
#include <cassert>

Value::Value()
    : Value{ Kind::INVALID, nullptr, nullptr }
{
}

bool Value::operator==(const Value& value) const
{
    // check the regular fields
    if (kind != value.kind || vslType != value.vslType ||
        llvmValue != value.llvmValue)
    {
        return false;
    }
    if (isField())
    {
        // check the base data
        return value.isField() && base.vslType == value.base.vslType &&
            base.llvmValue == value.base.llvmValue &&
            base.shouldDestroy == value.base.shouldDestroy;
    }
    // not a field but other checks succeeded
    return true;
}

bool Value::operator!=(const Value& value) const
{
    return !(*this == value);
}

Value Value::getNull()
{
    return {};
}

Value Value::getExpr(const Type* vslType, llvm::Value* llvmValue)
{
    return { Kind::EXPR, vslType, llvmValue };
}

Value Value::getVar(const Type* vslType, llvm::Value* llvmVar)
{
    return { Kind::VAR, vslType, llvmVar };
}

Value Value::getField(Value base, const Type* vslField, llvm::Value* llvmField,
    bool destroyBase)
{
    assert(base.isExpr() && "Not an expr!");
    Value value{ Kind::FIELD, vslField, llvmField };
    value.base = { base.getVSLType(), base.getLLVMValue(), destroyBase };
    return value;
}

Value Value::getFunc(const FunctionType* funcType, llvm::Function* llvmFunc)
{
    return { Kind::FUNC, funcType, llvmFunc };
}

bool Value::isValid() const
{
    return vslType && llvmValue && kind != Kind::INVALID;
}

bool Value::operator!() const
{
    return !isValid();
}

Value::operator bool() const
{
    return isValid();
}

bool Value::isAssignable() const
{
    return isVar() || isField();
}

bool Value::isExpr() const
{
    return kind == Kind::EXPR;
}

const Type* Value::getVSLType() const
{
    return vslType;
}

llvm::Value* Value::getLLVMValue() const
{
    return llvmValue;
}

bool Value::isVar() const
{
    return kind == Kind::VAR;
}

const Type* Value::getVSLVar() const
{
    assert(isVar() && "Not a var!");
    return vslType;
}

llvm::Value* Value::getLLVMVar() const
{
    assert(isVar() && "Not a var!");
    return llvmValue;
}

bool Value::isField() const
{
    return kind == Kind::FIELD;
}

const Type* Value::getVSLField() const
{
    assert(isField() && "Not a field!");
    return vslType;
}

llvm::Value* Value::getLLVMField() const
{
    assert(isField() && "Not a field!");
    return llvmValue;
}

Value Value::getBase() const
{
    return Value::getExpr(base.vslType, base.llvmValue);
}

bool Value::shouldDestroyBase() const
{
    assert(isField() && "Not a field!");
    return base.shouldDestroy;
}

bool Value::isFunc() const
{
    return kind == Kind::FUNC;
}

const FunctionType* Value::getVSLFunc() const
{
    assert(isFunc() && vslType->isFunctionType() && "Not a func!");
    return static_cast<const FunctionType*>(vslType);
}

llvm::Function* Value::getLLVMFunc() const
{
    assert(isFunc() && llvm::Function::classof(llvmValue) && "Not a func!");
    return static_cast<llvm::Function*>(llvmValue);
}

Value::Value(Kind kind, const Type* vslType, llvm::Value* llvmValue)
    : kind{ kind }, vslType{ vslType }, llvmValue{ llvmValue }
{
}
