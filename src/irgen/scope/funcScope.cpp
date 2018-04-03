#include "irgen/scope/funcScope.hpp"

FuncScope::FuncScope()
    : returnType{ nullptr }
{
}

Value FuncScope::get(llvm::StringRef name) const
{
    // go through each scope, starting at the most recent one
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
    // insert returns a pair<iterator, bool> where bool is true if successful,
    //  but we want the opposite of that according to our docs
    return !vars.back().insert(std::make_pair(name, value)).second;
}

void FuncScope::enter()
{
    vars.emplace_back();
}

void FuncScope::exit()
{
    vars.pop_back();
}

llvm::ArrayRef<FuncScope::VarItem> FuncScope::getVars() const
{
    const SymbolTable& table = vars.back();
    // mapvector iterators are guaranteed to be its vector type's iterator
    // since it's a std::vector, we can safely lower these to pointers because
    //  it has contiguous storage
    return llvm::makeArrayRef(&*table.begin(), &*table.end());
}

std::vector<llvm::ArrayRef<FuncScope::VarItem>> FuncScope::getAllVars() const
{
    std::vector<llvm::ArrayRef<VarItem>> scopes;
    scopes.reserve(vars.size());
    for (const SymbolTable& table : vars)
    {
        scopes.emplace_back(&*table.begin(), &*table.end());
    }
    return scopes;
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
