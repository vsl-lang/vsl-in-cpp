#ifndef SCOPETREE_HPP
#define SCOPETREE_HPP

#include "scope.hpp"
#include "type.hpp"
#include <string>
#include <vector>

class ScopeTree
{
public:
    ScopeTree();
    Type* get(const std::string& s) const;
    bool set(const std::string& s, Type* t);
    bool isGlobal() const;
    void enter();
    void enter(Type* returnType);
    void exit();
    Type* getReturnType();

private:
    std::vector<Scope> scopes;
};

#endif // SCOPETREE_HPP
