#ifndef TYPE_HPP
#define TYPE_HPP

#include <memory>
#include <string>
#include <vector>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"

class Type
{
public:
    enum Kind
    {
        ERROR,
        INT,
        FUNCTION,
        VOID
    };
    Type(Kind kind);
    virtual ~Type() = 0;
    static const char* kindToString(Type::Kind kind);
    virtual std::unique_ptr<Type> clone() const = 0;
    virtual std::string toString() const = 0;
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& context) const = 0;
    Kind kind;
};

class SimpleType : public Type
{
public:
    SimpleType(Type::Kind kind);
    virtual ~SimpleType() override = default;
    virtual std::unique_ptr<Type> clone() const override;
    virtual std::string toString() const override;
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& context) const override;
};

class FunctionType : public Type
{
public:
    FunctionType(std::vector<std::unique_ptr<Type>> params,
        std::unique_ptr<Type> returnType);
    virtual ~FunctionType() override = default;
    virtual std::unique_ptr<Type> clone() const override;
    virtual std::string toString() const override;
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& context) const override;
    std::vector<std::unique_ptr<Type>> params;
    std::unique_ptr<Type> returnType;
};

#endif // TYPE_HPP
