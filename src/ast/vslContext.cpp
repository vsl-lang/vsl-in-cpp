#include "ast/vslContext.hpp"
#include <iostream>

VSLContext::VSLContext()
    : errorType{ Type::ERROR }, boolType{ Type::BOOL }, intType{ Type::INT },
    voidType{ Type::VOID }
{
}

void VSLContext::addNode(std::unique_ptr<Node> node)
{
    nodes.push_back(std::move(node));
}

void VSLContext::setGlobal(DeclNode* decl)
{
    globals.push_back(decl);
}

llvm::ArrayRef<DeclNode*> VSLContext::getGlobals() const
{
    return globals;
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

const FunctionType* VSLContext::getFunctionType(FuncInterfaceNode& node)
{
    // copy only the param types
    std::vector<const Type*> paramTypes;
    paramTypes.resize(node.getNumParams());
    for (size_t i = 0; i < node.getNumParams(); ++i)
    {
        paramTypes[i] = node.getParam(i).getType();
    }
    return getFunctionType(std::move(paramTypes), node.getReturnType());
}
