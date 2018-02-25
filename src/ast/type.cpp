#include "ast/type.hpp"

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const Type& type)
{
    type.print(os);
    return os;
}

Type::Kind Type::getKind() const
{
    return kind;
}

bool Type::is(Kind kind) const
{
    return this->kind == kind;
}

bool Type::isNot(Kind kind) const
{
    return this->kind != kind;
}

bool Type::isFunctionType() const
{
    return kind == FUNCTION;
}

bool Type::isValid() const
{
    return kind != ERROR && kind != VOID;
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

SimpleType::SimpleType(Type::Kind kind)
    : Type{ kind }
{
}

void SimpleType::printImpl(llvm::raw_ostream& os) const
{
    os << getKindName(getKind());
}

bool FunctionType::operator==(const FunctionType& ft) const
{
    return params == ft.params && returnType == ft.returnType;
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
        for (size_t i = 0; i < params.size(); ++i)
        {
            os << ", ";
            params[i]->print(os);
        }
    }
    os << ") -> ";
    returnType->print(os);
}
