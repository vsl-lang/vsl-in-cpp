#include "scopetree.hpp"

ScopeTree::ScopeTree()
    : scopes{ Scope{ nullptr } }
{
}

Type* ScopeTree::get(const std::string& s) const
{
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
        Type* t = it->get(s);
        if (t != nullptr)
        {
            return t;
        }
    }
    return nullptr;
}

bool ScopeTree::set(const std::string& s, Type* t)
{
    return scopes.back().set(s, t);
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
