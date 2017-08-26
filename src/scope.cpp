#include "scope.hpp"

bool Scope::Item::operator==(const Item& rhs) const
{
    return type == rhs.type && value == rhs.value;
}

bool Scope::Item::operator!=(const Item& rhs) const
{
    return type != rhs.type || value != rhs.value;
}

Scope::Scope(Type* returnType)
    : returnType{ returnType }
{
}

Scope::Item Scope::get(const std::string& s) const
{
    auto it = symtab.find(s);
    if (it == symtab.end())
    {
        return { nullptr, nullptr };
    }
    return it->second;
}

bool Scope::set(const std::string& s, Item i)
{
    return symtab.emplace(s, i).second;
}

Type* Scope::getReturnType()
{
    return returnType;
}
