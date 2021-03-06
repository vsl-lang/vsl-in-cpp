#include "driver/driver.hpp"
#include "ast/nodePrinter.hpp"
#include "ast/vslContext.hpp"
#include "codegen/codegen.hpp"
#include "diag/diag.hpp"
#include "irgen/irgen.hpp"
#include "lexer/vslLexer.hpp"
#include "parser/vslParser.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

int Driver::main(int argc, const char* const* argv)
{
    op.parse(argc, argv);
    switch (op.action)
    {
    case OptionParser::DISPLAY_HELP:
        return displayHelp();
    case OptionParser::COMPILE:
        return compile();
    case OptionParser::REPL_LEX:
        return repl([](const std::string& in, llvm::raw_ostream& os)
            {
                Diag diag{ os };
                VSLLexer lexer{ diag, in.c_str() };
                Token token;
                do
                {
                    token = lexer.nextToken();
                    os << token << '\n';
                }
                while (token.isNot(TokenKind::END));
            });
    case OptionParser::REPL_PARSE:
        return repl([](const std::string& in, llvm::raw_ostream& os)
            {
                VSLContext vslCtx;
                Diag diag{ os };
                VSLLexer lexer{ diag, in.c_str() };
                VSLParser parser{ vslCtx, lexer };
                parser.parse();
                os << '\n';
                NodePrinter printer{ os };
                printer.visitAST(vslCtx.getGlobals());
            });
    case OptionParser::REPL_GENERATE:
        return repl([&](const std::string& in, llvm::raw_ostream& os)
            {
                VSLContext vslCtx;
                Diag diag{ os };
                // lex and parse the file
                VSLLexer lexer{ diag, in.c_str() };
                VSLParser parser{ vslCtx, lexer };
                parser.parse();
                // generate LLVM IR for the ast stored in vslCtx
                llvm::LLVMContext llvmContext;
                auto module = std::make_unique<llvm::Module>("repl",
                    llvmContext);
                // configure the module
                CodeGen codeGen{ diag, *module };
                codeGen.configure();
                // generate llvm ir
                IRGen irgen{ vslCtx, diag, *module };
                irgen.run();
                // possibly optimize the ir
                if (op.optimize)
                {
                    codeGen.optimize();
                }
                os << *module << '\n';
            });
    }
    return 0;
}

int Driver::displayHelp()
{
    llvm::outs() <<
        "Usage: vsl [options] [file]\n"
        "Options:\n"
        "  -h --help Display this information.\n"
        "  -o <file> Specify the output of compilation.\n"
        "  -O<level> Set optimization level (0 or 1).\n"
        "REPL Options:\n"
        "  -l        Start the lexer REPL.\n"
        "  -p        Start the parser REPL.\n"
        "  -g        Start the generator REPL.\n";
    return 0;
}

int Driver::compile()
{
    // open the input file
    Diag diag{ llvm::errs() };
    if (!op.infile)
    {
        diag.print<Diag::NO_INPUT>();
        return 1;
    }
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> in =
        llvm::MemoryBuffer::getFileOrSTDIN(op.infile);
    if (std::error_code ec = in.getError())
    {
        diag.print<Diag::CANT_OPEN_FILE>(op.infile, ec.message());
        return 1;
    }
    // lex/parse
    VSLContext vslCtx;
    VSLLexer lexer{ diag, in.get()->getBuffer().data() };
    VSLParser parser{ vslCtx, lexer };
    parser.parse();
    // configure llvm module
    llvm::LLVMContext llvmContext;
    auto module = std::make_unique<llvm::Module>(op.infile, llvmContext);
    CodeGen codeGen{ diag, *module };
    codeGen.configure();
    // emit llvm ir
    IRGen irgen{ vslCtx, diag, *module };
    irgen.run();
    // recap the amount of errors/warnings that occurred
    if (diag.getNumErrors() > 1)
    {
        llvm::errs() << diag.getNumErrors() << " errors generated\n";
    }
    if (diag.getNumWarnings() > 1)
    {
        llvm::errs() << diag.getNumWarnings() << " warnings generated\n";
    }
    if (diag.getNumErrors())
    {
        return 1;
    }
    // optimize ir
    if (op.optimize)
    {
        codeGen.optimize();
    }
    // open output file
    std::error_code ec;
    llvm::raw_fd_ostream out{ op.outfile, ec, llvm::sys::fs::F_None };
    if (ec)
    {
        diag.print<Diag::CANT_OPEN_FILE>(op.outfile, ec.message());
        return 1;
    }
    // emit object code
    codeGen.compile(out);
    return diag.getNumErrors() ? 1 : 0;
}

int Driver::repl(
    std::function<void(const std::string&, llvm::raw_ostream&)> evaluator)
{
    // std::cin is needed here because of its line-based input unlike
    //  llvm::MemoryBuffer
    std::string input;
    while (std::cout.good() && std::cin.good())
    {
        std::cout << "> ";
        std::getline(std::cin, input);
        evaluator(input, llvm::errs());
        input.clear();
    }
    return 0;
}
