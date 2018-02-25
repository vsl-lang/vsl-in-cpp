#include "irgen/typeConverter/typeConverter.hpp"

TypeConverter::TypeConverter(llvm::LLVMContext& llvmCtx)
    : llvmCtx{ llvmCtx }
{
}

llvm::Type* TypeConverter::convert(const Type* type) const
{
    switch (type->getKind())
    {
    case Type::VOID:
        return llvm::Type::getVoidTy(llvmCtx);
    case Type::BOOL:
        return llvm::Type::getInt1Ty(llvmCtx);
    case Type::INT:
        return llvm::Type::getInt32Ty(llvmCtx);
    case Type::FUNCTION:
        return convert(static_cast<const FunctionType*>(type));
    default:
        // opaque struct type
        return llvm::StructType::create(llvmCtx);
    }
}

llvm::Type* TypeConverter::convert(const SimpleType* type) const
{
    // delegate over to the switch above
    return convert(static_cast<const Type*>(type));
}

llvm::FunctionType* TypeConverter::convert(const FunctionType* type) const
{
    // copy converted parameter types
    std::vector<llvm::Type*> params;
    params.resize(type->getNumParams());
    for (size_t i = 0; i < type->getNumParams(); ++i)
    {
        params[i] = convert(type->getParamType(i));
    }
    // convert return type
    llvm::Type* ret = convert(type->getReturnType());
    // create the function type
    return llvm::FunctionType::get(ret, params, /*isVarArg=*/false);
}
