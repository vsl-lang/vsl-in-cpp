#include "irgen/scope/funcScope.hpp"

FuncScope::FuncScope()
    : returnType{ nullptr }
{
}

Value FuncScope::get(llvm::StringRef name) const
{
    // go through each scope
    for (auto it = vars.rbegin(); it != vars.rend(); ++it)
    {
        // if the variable was found, return it
        if (Value value = it->lookup(name))
        {
            return value;
        }
    }
    // otherwise, return a null Value
    return Value::getNull();
}

bool FuncScope::set(llvm::StringRef name, Value value)
{
    return !vars.back().try_emplace(name, value).second;
}

void FuncScope::enter()
{
    vars.emplace_back();
}

void FuncScope::exit()
{
    vars.pop_back();
}

bool FuncScope::empty() const
{
    return vars.empty();
}

const Type* FuncScope::getReturnType() const
{
    return returnType;
}

void FuncScope::setReturnType(const Type* returnType)
{
    this->returnType = returnType;
}
