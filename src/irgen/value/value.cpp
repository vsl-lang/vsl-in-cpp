#include "irgen/value/value.hpp"
#include <cassert>

Value::Value()
    : Value{ Kind::INVALID, nullptr, nullptr }
{
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
