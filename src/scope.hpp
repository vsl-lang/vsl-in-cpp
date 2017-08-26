#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "type.hpp"
#include "llvm/IR/Value.h"
#include <string>
#include <unordered_map>

class Scope
{
public:
    struct Item
    {
        bool operator==(const Item& rhs) const;
        bool operator!=(const Item& rhs) const;
        Type* type;
        llvm::Value* value;
    };
    Scope(Type* returnType);
    Item get(const std::string& s) const;
    bool set(const std::string& s, Item i);
    Type* getReturnType();
    
private:
    std::unordered_map<std::string, Item> symtab;
    Type* returnType;
};

#endif // SCOPE_HPP
