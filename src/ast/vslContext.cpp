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

const UnresolvedType* VSLContext::getUnresolvedType(llvm::StringRef name)
{
    return &*unresolvedTypes.insert(UnresolvedType{ name }).first;
}

const FunctionType* VSLContext::getFunctionType(const FuncInterfaceNode& node)
{
    // create the function type
    FunctionType type{ getParamTypes(node), node.getReturnType(),
        /*ctor=*/ node.is(Node::CTOR) };
    const ClassType* selfType;
    if (node.is(Node::METHOD))
    {
        selfType = static_cast<const MethodNode&>(node).getParent().getType();
    }
    else if (node.is(Node::CTOR))
    {
        selfType = static_cast<const CtorNode&>(node).getParent().getType();
    }
    type.setSelfType(selfType);
    return &*functionTypes.emplace(std::move(type)).first;
}

ClassType* VSLContext::createClassType(llvm::StringRef name)
{
    // try to insert a new empty ClassType
    auto* ptr = new ClassType{ name };
    auto pair = namedTypes.try_emplace(name, ptr);
    if (!pair.second)
    {
        // name already exists!
        return nullptr;
    }
    // if successful, return a pointer to the new ClassType
    return ptr;
}

const Type* VSLContext::getType(llvm::StringRef name) const
{
    auto it = namedTypes.find(name);
    if (it == namedTypes.end())
    {
        // name doesn't exist
        return nullptr;
    }
    // return a ptr to the type
    return it->getValue().get();
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
