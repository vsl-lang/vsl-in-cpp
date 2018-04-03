#include "irgen/scope/globalScope.hpp"

Value GlobalScope::get(llvm::StringRef name) const
{
    return symtab.lookup(name);
}

std::pair<Value, Access> GlobalScope::getCtor(const Type* type) const
{
    auto it = ctors.find(type);
    if (it == ctors.end())
    {
        // type doesn't even have a ctor
        return { Value::getNull(), Access::NONE };
    }
    return it->second;
}

std::pair<Value, Access> GlobalScope::getMethod(const Type* type,
    llvm::StringRef name) const
{
    auto it = methods.find(type);
    if (it == methods.end())
    {
        // type doesn't even have any methods
        return { Value::getNull(), Access::NONE };
    }
    // lookup the method
    return it->second.lookup(name);
}

llvm::Function* GlobalScope::getDtor(const Type* type) const
{
    auto it = dtors.find(type);
    if (it == dtors.end())
    {
        return nullptr;
    }
    return it->second;
}

bool GlobalScope::setFunc(llvm::StringRef name, const FunctionType* type,
    llvm::Function* func)
{
    // try_emplace returns pair<iterator, bool> where bool is true if successful
    // we want the opposite of that bool
    return !symtab.try_emplace(name, Value::getFunc(type, func)).second;
}

bool GlobalScope::setVar(llvm::StringRef name, const Type* type,
    llvm::Value* var)
{
    return !symtab.try_emplace(name, Value::getVar(type, var)).second;
}

bool GlobalScope::setCtor(const Type* type, const FunctionType* vslFunc,
    llvm::Function* llvmFunc, Access access)
{
    assert(vslFunc->isCtor() && "not a ctor!");
    return !ctors.emplace(type,
        std::make_pair(Value::getFunc(vslFunc, llvmFunc), access)).second;
}

bool GlobalScope::setMethod(const Type* type, llvm::StringRef name,
    const FunctionType* vslFunc, llvm::Function* llvmFunc, Access access)
{
    assert(vslFunc->isMethod() && "not a method!");
    return !methods[type].try_emplace(name,
        Value::getFunc(vslFunc, llvmFunc), access).second;
}

bool GlobalScope::setDtor(const Type* type, llvm::Function* llvmFunc)
{
    return !dtors.emplace(type, llvmFunc).second;
}
