#include "irgen/passes/typeResolver/typeResolver.hpp"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/Casting.h"
#include <string>

TypeResolver::TypeResolver(VSLContext& vslCtx, TypeConverter& converter,
    llvm::Module& module)
    : vslCtx{ vslCtx }, converter{ converter }, module{ module },
    llvmCtx{ module.getContext() }
{
}

void TypeResolver::visitAST(llvm::ArrayRef<DeclNode*> ast)
{
    // pass 1: gather info on class names and create reference types
    pass = PASS_1;
    NodeVisitor::visitAST(ast);
    // pass 2: resolve fields and construct class types
    pass = PASS_2;
    NodeVisitor::visitAST(ast);
}

void TypeResolver::visitClass(ClassNode& node)
{
    switch (pass)
    {
    case PASS_1:
        gatherInfo(node);
        break;
    case PASS_2:
        resolve(node);
        break;
    }
}

void TypeResolver::gatherInfo(ClassNode& node)
{
    const ClassType* vslType = node.getClassType();
    // create a named llvm struct type to hold all the fields
    // the "struct.<class>" prefix prevents collisions with the reference type
    auto* structType = llvm::StructType::create(llvmCtx,
        std::string{ "struct." } += node.getName());
    // register the class' llvm equivalent
    converter.addClassType(node.getName(), vslType, structType);
}

void TypeResolver::resolve(ClassNode& node)
{
    const ClassType* classType = node.getClassType();
    // get the llvm struct type
    // the representation of this is documented in TypeConverter::addClassType
    llvm::PointerType* ptrType = converter.convert(classType);
    llvm::StructType* rcType =
        llvm::cast<llvm::StructType>(ptrType->getElementType());
    // (rcType has the 0th field as an i32 for the refcount)
    llvm::StructType* structType =
        llvm::cast<llvm::StructType>(rcType->getElementType(1));
    // fill in the structType with the llvm equivalent of each field type
    std::vector<llvm::Type*> llvmFieldTypes;
    llvmFieldTypes.resize(node.getNumFields());
    for (size_t i = 0; i < node.getNumFields(); ++i)
    {
        llvmFieldTypes[i] = converter.convert(node.getField(i).getType());
    }
    structType->setBody(llvmFieldTypes);
}
