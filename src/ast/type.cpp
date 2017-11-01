#include "ast/type.hpp"
#include "llvm/IR/DerivedTypes.h"
#include <algorithm>

Type::Type(Kind kind)
    : kind{ kind }
{
}

Type::~Type()
{
}

const char* Type::kindToString(Type::Kind kind)
{
    switch (kind)
    {
    case ERROR:
        return "ErrorType";
    case BOOL:
        return "Bool";
    case INT:
        return "Int";
    case VOID:
        return "Void";
    default:
        return "InvalidType";
    }
}

SimpleType::SimpleType(Type::Kind kind)
    : Type{ kind }
{
}

std::unique_ptr<Type> SimpleType::clone() const
{
    return std::make_unique<SimpleType>(kind);
}

std::string SimpleType::toString() const
{
    return kindToString(kind);
}

llvm::Type* SimpleType::toLLVMType(llvm::LLVMContext& context) const
{
    switch (kind)
    {
    case Type::BOOL:
        return llvm::Type::getInt1Ty(context);
    case Type::INT:
        return llvm::Type::getInt32Ty(context);
    case Type::VOID:
        return llvm::Type::getVoidTy(context);
    default:
        return nullptr;
    }
}

FunctionType::FunctionType(std::vector<std::unique_ptr<Type>> params,
    std::unique_ptr<Type> returnType)
    : Type{ Type::FUNCTION }, params{ std::move(params) },
    returnType{ std::move(returnType) }
{
}

std::unique_ptr<Type> FunctionType::clone() const
{
    std::vector<std::unique_ptr<Type>> newParams;
    newParams.reserve(params.size());
    std::unique_ptr<Type> newReturnType = returnType->clone();
    // this is actually just an excuse to use the cool c++11 functions instead
    //  of a boring old for loop
    std::transform(params.begin(), params.end(), std::back_inserter(newParams),
        [](const std::unique_ptr<Type>& param)
        {
            return param->clone();
        });
    return std::make_unique<FunctionType>(std::move(newParams),
        std::move(newReturnType));
}

std::string FunctionType::toString() const
{
    std::string s = "(";
    if (!params.empty())
    {
        s += params[0]->toString();
        for (auto it = params.begin() + 1; it != params.end(); ++it)
        {
            s += ", ";
            s += (*it)->toString();
        }
    }
    s += ") -> ";
    s += returnType->toString();
    return s;
}

llvm::Type* FunctionType::toLLVMType(llvm::LLVMContext& context) const
{
    std::vector<llvm::Type*> llvmParams;
    llvmParams.resize(params.size());
    std::transform(params.begin(), params.end(), llvmParams.begin(),
        [&](const auto& param)
        {
            return param->toLLVMType(context);
        });
    llvm::Type* llvmReturnType = returnType->toLLVMType(context);
    return llvm::FunctionType::get(llvmReturnType, std::move(llvmParams),
        false);
}
