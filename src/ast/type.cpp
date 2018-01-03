#include "ast/type.hpp"
#include "llvm/IR/DerivedTypes.h"
#include <algorithm>
#include <iterator>

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const Type& type)
{
    type.print(os);
    return os;
}

bool Type::isValid() const
{
    return kind != ERROR && kind != VOID;
}

bool Type::isFunctionType() const
{
    return kind == FUNCTION;
}

void Type::print(llvm::raw_ostream& os) const
{
    printImpl(os);
}

Type::Type(Kind kind)
    : kind{ kind }
{
}

const char* Type::getKindName(Kind k)
{
    switch (k)
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

Type::Kind Type::getKind() const
{
    return kind;
}

llvm::Type* SimpleType::toLLVMType(llvm::LLVMContext& context) const
{
    switch (getKind())
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

SimpleType::SimpleType(Type::Kind kind)
    : Type{ kind }
{
}

void SimpleType::printImpl(llvm::raw_ostream& os) const
{
    os << getKindName(getKind());
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
    return llvm::FunctionType::get(llvmReturnType, llvmParams, false);
}

bool FunctionType::operator==(const FunctionType& ft) const
{
    return returnType == ft.returnType && params.size() == ft.params.size() &&
        std::equal(params.begin(), params.end(), ft.params.begin());
}

size_t FunctionType::getNumParams() const
{
    return params.size();
}

const Type* FunctionType::getParamType(size_t i) const
{
    return params[i];
}

llvm::ArrayRef<const Type*> FunctionType::getParams() const
{
    return params;
}

const Type* FunctionType::getReturnType() const
  {
    return returnType;
}

FunctionType::FunctionType(std::vector<const Type*> params,
    const Type* returnType)
    : Type{ Type::FUNCTION }, params{ std::move(params) },
    returnType{ returnType }
{
}

void FunctionType::printImpl(llvm::raw_ostream& os) const
{
    os << '(';
    if (!params.empty())
    {
        params[0]->print(os);
        for (auto it = std::next(params.begin()); it != params.end(); ++it)
        {
            os << ", ";
            (*it)->print(os);
        }
    }
    os << ") -> ";
    returnType->print(os);
}
