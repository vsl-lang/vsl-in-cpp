#include "codegen/codegen.hpp"
#include "diag/diag.hpp"
#include "driver/driver.hpp"
#include "irgen/irgen.hpp"
#include "lexer/vsllexer.hpp"
#include "parser/vslparser.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include <cstring>
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
                VSLContext vslContext;
                Diag diag{ os };
                VSLLexer lexer{ diag, in.c_str() };
                VSLParser parser{ vslContext, lexer };
                os << parser.parse()->toString() << '\n';
            });
    case OptionParser::REPL_GENERATE:
        return repl([&](const std::string& in, llvm::raw_ostream& os)
            {
                VSLContext vslContext;
                Diag diag{ os };
                VSLLexer lexer{ diag, in.c_str() };
                VSLParser parser{ vslContext, lexer };
                auto ast = parser.parse();
                llvm::LLVMContext llvmContext;
                auto module = std::make_unique<llvm::Module>("repl",
                    llvmContext);
                IRGen irGen{ vslContext, diag, *module };
                ast->accept(irGen);
                CodeGen codeGen{ diag, *module };
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
    VSLContext vslContext;
    VSLLexer lexer{ diag, in.get()->getBuffer().data() };
    VSLParser parser{ vslContext, lexer };
    auto ast = parser.parse();
    // emit llvm ir
    llvm::LLVMContext llvmContext;
    auto module = std::make_unique<llvm::Module>(op.infile, llvmContext);
    IRGen irGen{ vslContext, diag, *module };
    ast->accept(irGen);
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
    CodeGen codeGen{ diag, *module };
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
