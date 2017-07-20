#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "type.hpp"
#include <string>
#include <unordered_map>

class Scope
{
public:
    Scope(Type* returnType);
    Type* get(const std::string& s) const;
    bool set(const std::string& s, Type* t);
    Type* getReturnType();
    
private:
    std::unordered_map<std::string, Type*> symtab;
    Type* returnType;
};

#endif // SCOPE_HPP
