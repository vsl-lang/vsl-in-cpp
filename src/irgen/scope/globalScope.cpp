#include "irgen/scope/globalScope.hpp"

Value GlobalScope::get(llvm::StringRef name) const
{
    return symtab.lookup(name);
}

bool GlobalScope::setFunc(llvm::StringRef name, const FunctionType* type,
    llvm::Function* func)
{
    return !symtab.try_emplace(name, Value::getFunc(type, func)).second;
}

bool GlobalScope::setVar(llvm::StringRef name, const Type* type,
    llvm::Value* var)
{
    return !symtab.try_emplace(name, Value::getVar(type, var)).second;
}
