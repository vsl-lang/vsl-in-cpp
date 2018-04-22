#include "ast/type.hpp"
#include "ast/vslContext.hpp"

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

const UnresolvedType* Type::asUnresolvedType() const
{
    return is(Type::UNRESOLVED) ? static_cast<const UnresolvedType*>(this)
        : nullptr;
}

const FunctionType* Type::asFunctionType() const
{
    return is(Type::FUNCTION) ? static_cast<const FunctionType*>(this)
        : nullptr;
}

const ClassType* Type::asClassType() const
{
    return is(Type::CLASS) ? static_cast<const ClassType*>(this) : nullptr;
}

bool Type::isValid() const
{
    return kind != ERROR && kind != VOID;
}

bool Type::matches(const Type* type, VSLContext& vslCtx) const
{
    return resolve(vslCtx) == type->resolve(vslCtx);
}

const Type* Type::resolve(VSLContext& vslCtx) const
{
    return resolveImpl(vslCtx);
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

const Type* Type::resolveImpl(VSLContext& vslCtx) const
{
    // assumed if resolveImpl isn't overridden
    return this;
}

SimpleType::SimpleType(Type::Kind kind)
    : Type{ kind }
{
}

void SimpleType::printImpl(llvm::raw_ostream& os) const
{
    os << getKindName(getKind());
}

bool UnresolvedType::operator==(const UnresolvedType& type) const
{
    return name == type.name;
}

llvm::StringRef UnresolvedType::getName() const
{
    return name;
}

UnresolvedType::UnresolvedType(llvm::StringRef name)
    : Type{ Type::UNRESOLVED }, name{ name }, actualType{ nullptr }
{
}

const Type* UnresolvedType::resolveImpl(VSLContext& vslCtx) const
{
    if (actualType)
    {
        // already called before so return cached type
        return actualType;
    }
    // attempt to lookup the name
    actualType = vslCtx.getType(name);
    if (!actualType)
    {
        // type doesn't exist, fallback to superclass implementation
        actualType = Type::resolveImpl(vslCtx);
    }
    return actualType;
}

void UnresolvedType::printImpl(llvm::raw_ostream& os) const
{
    os << name;
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

const ClassType* FunctionType::getSelfType() const
{
    return selfType;
}

void FunctionType::setSelfType(const ClassType* selfType)
{
    this->selfType = selfType;
}

FunctionType::FunctionType(std::vector<const Type*> params,
    const Type* returnType, bool ctor)
    : Type{ Type::FUNCTION }, params{ std::move(params) },
    returnType{ returnType }, selfType{ nullptr }, ctor{ ctor }
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

llvm::StringRef ClassType::getName() const
{
    return name;
}

ClassType::ClassType(llvm::StringRef name)
    : Type{ Type::CLASS }, name{ name }
{
}

void ClassType::printImpl(llvm::raw_ostream& os) const
{
    os << name;
}
