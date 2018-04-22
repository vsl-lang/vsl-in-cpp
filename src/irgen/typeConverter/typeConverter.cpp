#include "irgen/typeConverter/typeConverter.hpp"

TypeConverter::TypeConverter(VSLContext& vslCtx, llvm::LLVMContext& llvmCtx)
    : vslCtx{ vslCtx }, llvmCtx{ llvmCtx }
{
}

llvm::Type* TypeConverter::convert(const Type* type) const
{
    if (!type)
    {
        // type doesn't even exist
        return getOpaqueType();
    }
    switch (type->getKind())
    {
    case Type::VOID:
        return llvm::Type::getVoidTy(llvmCtx);
    case Type::BOOL:
        return llvm::Type::getInt1Ty(llvmCtx);
    case Type::INT:
        return llvm::Type::getInt32Ty(llvmCtx);
    case Type::UNRESOLVED:
        return convert(static_cast<const UnresolvedType*>(type));
    case Type::FUNCTION:
        return convert(static_cast<const FunctionType*>(type));
    case Type::CLASS:
        return convert(static_cast<const ClassType*>(type));
    default:
        // invalid type
        return getOpaqueType();
    }
}

llvm::Type* TypeConverter::convert(const UnresolvedType* type) const
{
    // attempt to lookup the type
    const Type* resolved = type->resolve(vslCtx);
    if (type == resolved)
    {
        // trying to resolve returns the same type so it's forever indefinite
        return getOpaqueType();
    }
    return convert(resolved);
}

llvm::FunctionType* TypeConverter::convert(const FunctionType* type) const
{
    std::vector<llvm::Type*> params;
    llvm::Type* ret;
    if (type->hasSelfType())
    {
        // ctors/methods require an implicit self parameter in LLVM but not VSL
        params.resize(type->getNumParams() + 1);
        params[0] = convert(type->getSelfType());
        // copy converted parameter types
        for (size_t i = 0; i < type->getNumParams(); ++i)
        {
            params[i + 1] = convert(type->getParamType(i));
        }
        if (type->isCtor())
        {
            // the actual constructor returns null, since that just initializes
            //  an object in a manner similar to a method
            ret = llvm::Type::getVoidTy(llvmCtx);
        }
        else
        {
            ret = convert(type->getReturnType());
        }
    }
    else
    {
        params.resize(type->getNumParams());
        // copy converted parameter types
        for (size_t i = 0; i < type->getNumParams(); ++i)
        {
            params[i] = convert(type->getParamType(i));
        }
        // convert return type
        ret = convert(type->getReturnType());
    }
    // create the function type
    return llvm::FunctionType::get(ret, params, /*isVarArg=*/false);
}

llvm::PointerType* TypeConverter::convert(const ClassType* type) const
{
    // look for the llvm reference type from previous calls to addClassType
    auto it = classes.find(type);
    if (it == classes.end())
    {
        // could not find the class type
        return llvm::PointerType::getUnqual(getOpaqueType());
    }
    // get the reference type
    return it->second;
}

void TypeConverter::addClassType(llvm::StringRef name, const ClassType* vslType)
{
    // create a named llvm struct type to hold all the fields
    // the "struct.<class>" prefix prevents collisions with the final ref type
    auto* structType = llvm::StructType::create(llvmCtx,
        std::string{ "struct." } += name);
    // same as structType but with a reference count in the front
    auto* objType = llvm::StructType::create(name,
        llvm::Type::getInt32Ty(llvmCtx), structType);
    // pointer to the reference-counted objType
    auto* refType = llvm::PointerType::getUnqual(objType);
    // insert the class/reference types (result is pair<iterator, bool>)
    auto pair = classes.emplace(vslType, refType);
    // make sure that the insert was successful
    assert(pair.second && "Class already exists!");
}

llvm::StructType* TypeConverter::getOpaqueType() const
{
    return llvm::StructType::get(llvmCtx);
}
