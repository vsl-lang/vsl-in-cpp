#include "irgen/scope/funcScope.hpp"

VarItem::VarItem()
    : type{ nullptr }, value{ nullptr }
{
}

VarItem::VarItem(const Type* type, llvm::Value* value)
    : type{ type }, value{ value }
{
}

const Type* VarItem::getVSLType() const
{
    return type;
}

llvm::Value* VarItem::getLLVMValue() const
{
    return value;
}

bool VarItem::isValid() const
{
    return type && value;
}

VarItem::operator bool() const
{
    return isValid();
}

FuncScope::FuncScope()
    : returnType{ nullptr }
{
}

VarItem FuncScope::get(llvm::StringRef name) const
{
    // go through each scope
    for (auto it = vars.rbegin(); it != vars.rend(); ++it)
    {
        // if the variable was found, return it
        if (VarItem var = it->lookup(name))
        {
            return var;
        }
    }
    // otherwise, default-construct an invalid VarItem
    return {};
}

bool FuncScope::set(llvm::StringRef name, const Type* type, llvm::Value* value)
{
    return !vars.back().try_emplace(name, type, value).second;
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
