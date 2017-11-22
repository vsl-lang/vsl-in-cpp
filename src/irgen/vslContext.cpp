#include "irgen/vslContext.hpp"

VSLContext::VSLContext()
    : errorType{ Type::ERROR }, boolType{ Type::BOOL }, intType{ Type::INT },
    voidType{ Type::VOID }
{
}

const SimpleType* VSLContext::getBoolType() const
{
    return &boolType;
}

const SimpleType* VSLContext::getIntType() const
{
    return &intType;
}

const SimpleType* VSLContext::getVoidType() const
{
    return &voidType;
}

const SimpleType* VSLContext::getErrorType() const
{
    return &errorType;
}

const SimpleType* VSLContext::getSimpleType(Type::Kind k) const
{
    switch (k)
    {
    case Type::BOOL:
        return getBoolType();
    case Type::INT:
        return getIntType();
    case Type::VOID:
        return getVoidType();
    default:
        return getErrorType();
    }
}

const FunctionType* VSLContext::getFunctionType(std::vector<const Type*> params,
    const Type* returnType)
{
    // can't construct the FunctionType by emplace() because ctor is protected
    FunctionType ft{ std::move(params), returnType };
    return &*functionTypes.emplace(std::move(ft)).first;
}
