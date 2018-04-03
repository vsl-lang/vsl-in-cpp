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

bool NamedType::operator==(const NamedType& type) const
{
    return name == type.name;
}

llvm::StringRef NamedType::getName() const
{
    return name;
}

bool NamedType::hasUnderlyingType() const
{
    return underlyingType;
}

const Type* NamedType::getUnderlyingType() const
{
    return underlyingType;
}

void NamedType::setUnderlyingType(const Type* type) const
{
    underlyingType = type;
}

NamedType::NamedType(llvm::StringRef name)
    : Type{ Type::NAMED }, name{ name }, underlyingType{ nullptr }
{
}

void NamedType::printImpl(llvm::raw_ostream& os) const
{
    os << name;
    printUnderlyingType(os);
}

void NamedType::printUnderlyingType(llvm::raw_ostream& os) const
{
    if (hasUnderlyingType())
    {
        switch (underlyingType->getKind())
        {
        case Type::ERROR:
        case Type::VOID:
        case Type::BOOL:
        case Type::INT:
        case Type::FUNCTION:
            os << " (aka " << *underlyingType << ')';
            break;
        case Type::NAMED:
            // instead of getting something like "A (aka B (aka Int))" we
            //  attempt to print ITS underlying type so we get "A (aka Int)"
            // if it doesn't have an underlying type we'll just get "A" rather
            //  than "A (aka B)" which wouldn't really help much
            static_cast<const NamedType*>(underlyingType)->printUnderlyingType(
                os);
            break;
        case Type::CLASS:
            // class types don't really have a string representation for now
            break;
        }
    }
}

bool FunctionType::operator==(const FunctionType& type) const
{
    return ctor == type.ctor && selfType == type.selfType &&
        returnType == type.returnType && params == type.params;
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

bool FunctionType::isCtor() const
{
    return ctor;
}

bool FunctionType::isMethod() const
{
    return !isCtor() && hasSelfType();
}

bool FunctionType::hasSelfType() const
{
    return selfType;
}

const NamedType* FunctionType::getSelfType() const
{
    return selfType;
}

void FunctionType::setSelfType(const NamedType* selfType)
{
    this->selfType = selfType;
}

FunctionType::FunctionType(std::vector<const Type*> params,
    const Type* returnType, bool ctor)
    : Type{ Type::FUNCTION }, params{ std::move(params) },
    returnType{ returnType }, selfType{ nullptr },
    ctor{ ctor }
{
}

void FunctionType::printImpl(llvm::raw_ostream& os) const
{
    os << '(';
    if (!params.empty())
    {
        params[0]->print(os);
        for (size_t i = 1; i < params.size(); ++i)
        {
            os << ", ";
            params[i]->print(os);
        }
    }
    os << ") -> ";
    returnType->print(os);
}

ClassType::Field::Field()
    : Field{ nullptr, 0, Access::NONE }
{
}

ClassType::Field::Field(const Type* type, size_t index, Access access)
    : type{ type }, index{ index }, access{ access }
{
}

bool ClassType::Field::isValid() const
{
    return type && access != Access::NONE;
}

ClassType::Field::operator bool() const
{
    return isValid();
}

bool ClassType::Field::operator!() const
{
    return !isValid();
}

ClassType::Field ClassType::getField(llvm::StringRef name) const
{
    auto it = fieldTypes.find(name);
    if (it == fieldTypes.end())
    {
        return {};
    }
    return it->getValue();
}

bool ClassType::setField(llvm::StringRef name, const Type* type, size_t index,
    Access access)
{
    // try_emplace returns pair<iterator, bool>
    // the bool part is true if successful but we want the opposite of that
    return !fieldTypes.try_emplace(name, type, index, access).second;
}

ClassType::ClassType()
    : Type{ Type::CLASS }
{
}

void ClassType::printImpl(llvm::raw_ostream& os) const
{
    os << "<class type>";
}
