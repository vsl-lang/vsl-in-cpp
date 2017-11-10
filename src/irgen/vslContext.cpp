#include "irgen/vslContext.hpp"

VSLContext::VSLContext()
    : errorType{ Type::ERROR }, boolType{ Type::BOOL }, intType{ Type::INT },
    voidType{ Type::VOID }
{
}

const SimpleType* VSLContext::getSimpleType(Type::Kind k)
{
    switch (k)
    {
    case Type::BOOL:
        return &boolType;
    case Type::INT:
        return &intType;
    case Type::VOID:
        return &voidType;
    default:
        return &errorType;
    }
}

const FunctionType* VSLContext::getFunctionType(std::vector<const Type*> params,
    const Type* returnType)
{
    // can't construct the FunctionType by emplace() because ctor is protected
    FunctionType ft{ std::move(params), returnType };
    return &*functionTypes.emplace(std::move(ft)).first;
}
