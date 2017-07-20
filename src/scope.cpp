#include "scope.hpp"

Scope::Scope(Type* returnType)
    : returnType{ returnType }
{
}

Type* Scope::get(const std::string& s) const
{
    auto it = symtab.find(s);
    if (it == symtab.end())
    {
        return nullptr;
    }
    return it->second;
}

bool Scope::set(const std::string& s, Type* t)
{
    return symtab.emplace(s, t).second;
}

Type* Scope::getReturnType()
{
    return returnType;
}
