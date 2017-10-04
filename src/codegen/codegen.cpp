#include "codegen.hpp"
#include "llvm/ADT/Optional.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

CodeGen::CodeGen(llvm::Module& module, std::ostream& errors)
    : module{ module }, fpm{ &module }, errors{ errors }, errored{ false }
{
}

void CodeGen::compile(llvm::raw_pwrite_stream& output)
{
    // configure the compilation target
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    std::string targetTriple{ llvm::sys::getDefaultTargetTriple() };
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(
        targetTriple, error);
    if (target == nullptr)
    {
        errors << "error: couldn't find requested target: " << error << '\n';
        errored = true;
        return;
    }
    const char* cpu = "generic";
    const char* features = "";
    llvm::TargetOptions options;
    llvm::Optional<llvm::Reloc::Model> rm;
    llvm::TargetMachine* targetMachine = target->createTargetMachine(
        targetTriple, cpu, features, options, rm);
    // configure the module
    module.setDataLayout(targetMachine->createDataLayout());
    module.setTargetTriple(targetTriple);
    // compile the module
    if (targetMachine->addPassesToEmitFile(pm, output,
            llvm::TargetMachine::CGFT_ObjectFile))
    {
        errors << "error: target machine cannot emit a file of type object\n";
        errored = true;
        return;
    }
    pm.run(module);
    output.flush();
}

bool CodeGen::hasError() const
{
    return errored;
}

void CodeGen::optimize(int optLevel)
{
    auto passes =
    {
        llvm::createInstructionCombiningPass(),
        llvm::createReassociatePass(),
        llvm::createGVNPass(),
        llvm::createCFGSimplificationPass()
    };
    for (const auto& pass : passes)
    {
        fpm.add(pass);
    }
    fpm.doInitialization();
    for (auto& function : module.functions())
    {
        fpm.run(function);
    }
    fpm.doFinalization();
}
