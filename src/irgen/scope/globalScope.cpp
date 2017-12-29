#include "irgen/scope/globalScope.hpp"

FuncItem::FuncItem()
    : type{ nullptr }, func{ nullptr }
{
}

FuncItem::FuncItem(const FunctionType* type, llvm::Function* func)
    : type{ type }, func{ func }
{
}

const FunctionType* FuncItem::getVSLType() const
{
    return type;
}

llvm::Function* FuncItem::getLLVMFunc() const
{
    return func;
}

bool FuncItem::isValid() const
{
    return type && func;
}

FuncItem::operator bool() const
{
    return isValid();
}

FuncItem GlobalScope::getFunc(llvm::StringRef name) const
{
    return funcs.lookup(name);
}

bool GlobalScope::setFunc(llvm::StringRef name, const FunctionType* type,
    llvm::Function* func)
{
    return !funcs.try_emplace(name, type, func).second;
}
