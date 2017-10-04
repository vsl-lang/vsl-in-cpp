#include "scopetree.hpp"

ScopeTree::ScopeTree()
    : scopes{ Scope{ nullptr } }
{
}

Scope::Item ScopeTree::get(const std::string& s) const
{
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
        Scope::Item i = it->get(s);
        if (i.type != nullptr && i.value != nullptr)
        {
            return i;
        }
    }
    return { nullptr, nullptr };
}

bool ScopeTree::set(const std::string& s, Scope::Item i)
{
    return scopes.back().set(s, i);
}

bool ScopeTree::isGlobal() const
{
    return scopes.size() == 1;
}

void ScopeTree::enter()
{
    enter(getReturnType());
}

void ScopeTree::enter(Type* returnType)
{
    scopes.emplace_back(returnType);
}

void ScopeTree::exit()
{
    scopes.pop_back();
}

Type* ScopeTree::getReturnType()
{
    return scopes.back().getReturnType();
}
