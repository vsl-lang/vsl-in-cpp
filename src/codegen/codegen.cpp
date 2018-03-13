#include "codegen/codegen.hpp"
#include "llvm/ADT/Optional.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

CodeGen::CodeGen(Diag& diag, llvm::Module& module)
    : diag{ diag }, module{ module }, fpm{ &module }
{
}

void CodeGen::configure()
{
    // initialize target machines
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    // find out what target we're generating code for
    std::string targetTriple{ llvm::sys::getDefaultTargetTriple() };
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(
        targetTriple, error);
    if (!target)
    {
        diag.print<Diag::CANT_FIND_TARGET>(std::move(error));
        return;
    }
    // get the target machine details
    const char* cpu = "generic";
    const char* features = "";
    llvm::TargetOptions options;
    llvm::Optional<llvm::Reloc::Model> rm;
    machine = target->createTargetMachine(targetTriple, cpu, features,
        options, rm);
    // configure the module with this new info
    module.setDataLayout(machine->createDataLayout());
    module.setTargetTriple(targetTriple);
}

void CodeGen::compile(llvm::raw_pwrite_stream& output)
{
    // compile the module
    if (machine->addPassesToEmitFile(pm, output,
            llvm::TargetMachine::CGFT_ObjectFile))
    {
        diag.print<Diag::TARGET_CANT_EMIT_OBJ>();
        return;
    }
    pm.run(module);
    output.flush();
}

void CodeGen::optimize()
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
