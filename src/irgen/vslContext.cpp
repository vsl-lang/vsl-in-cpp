#include "irgen/vslContext.hpp"
#include <iostream>

VSLContext::VSLContext(llvm::raw_ostream& errors)
    : errorType{ Type::ERROR }, boolType{ Type::BOOL }, intType{ Type::INT },
    voidType{ Type::VOID }, errors{ errors }, errorCount{ 0 }
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

llvm::raw_ostream& VSLContext::error()
{
    ++errorCount;
    errors.changeColor(llvm::raw_ostream::RED, true) << "error:";
    errors.resetColor() << ' ';
    return errors;
}

llvm::raw_ostream& VSLContext::error(Location location)
{
    errors.changeColor(llvm::raw_ostream::SAVEDCOLOR, true) << location << ':';
    errors.resetColor() << ' ';
    return error();
}

size_t VSLContext::getErrorCount() const
{
    return errorCount;
}
