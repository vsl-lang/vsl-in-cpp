#include "ast/vslContext.hpp"

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

bool VSLContext::hasNamedType(llvm::StringRef name) const
{
    NamedType nt{ name };
    return namedTypes.find(nt) != namedTypes.end();
}

const NamedType* VSLContext::getNamedType(llvm::StringRef name)
{
    NamedType nt{ name };
    return &*namedTypes.insert(nt).first;
}

const FunctionType* VSLContext::getFunctionType(const FuncInterfaceNode& node)
{
    // create the function type
    FunctionType type{ getParamTypes(node), node.getReturnType(),
        node.is(Node::CTOR) };
    if (node.is(Node::METHOD))
    {
        // fill in the type of the self parameter for methods
        type.setSelfType(
            static_cast<const MethodNode&>(node).getParent().getType());
    }
    else if (node.is(Node::CTOR))
    {
        // fill in the type of the self parameter for ctors
        type.setSelfType(
            static_cast<const CtorNode&>(node).getParent().getType());
    }
    return &*functionTypes.emplace(std::move(type)).first;
}

const NamedType* VSLContext::createNamedType(llvm::StringRef name)
{
    // attempt to insert a NamedType
    NamedType nt{ name };
    auto pair = namedTypes.insert(nt);
    if (!pair.second)
    {
        // name already exists!
        return nullptr;
    }
    return &*pair.first;
}

ClassType* VSLContext::createClassType()
{
    // create a new empty ClassType and return a pointer to it
    classTypes.push_back({});
    return &classTypes.back();
}

std::vector<const Type*> VSLContext::getParamTypes(
    const FuncInterfaceNode& node)
{
    std::vector<const Type*> paramTypes;
    paramTypes.resize(node.getNumParams());
    for (size_t i = 0; i < node.getNumParams(); ++i)
    {
        paramTypes[i] = node.getParam(i).getType();
    }
    return paramTypes;
}
