#include "codegen/codegen.hpp"
#include "driver/driver.hpp"
#include "irgen/irgen.hpp"
#include "lexer/vsllexer.hpp"
#include "parser/vslparser.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include <cstring>
#include <fstream>
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
        return repl([](const std::string& in, llvm::raw_ostream& out)
            {
                VSLContext vslContext;
                VSLLexer lexer{ vslContext, in.c_str() };
                Token token;
                do
                {
                    token = lexer.nextToken();
                    out << token << " at " << token.location << '\n';
                }
                while (token.kind != TokenKind::END);
            });
    case OptionParser::REPL_PARSE:
        return repl([](const std::string& in, llvm::raw_ostream& out)
            {
                VSLContext vslContext;
                VSLLexer lexer{ vslContext, in.c_str() };
                VSLParser parser{ vslContext, lexer };
                out << parser.parse()->toString() << '\n';
            });
    case OptionParser::REPL_GENERATE:
        return repl([&](const std::string& in, llvm::raw_ostream& out)
            {
                VSLContext vslContext;
                VSLLexer lexer{ vslContext, in.c_str() };
                VSLParser parser{ vslContext, lexer };
                auto ast = parser.parse();
                if (!vslContext.getErrorCount())
                {
                    llvm::LLVMContext llvmContext;
                    auto module = std::make_unique<llvm::Module>("repl",
                        llvmContext);
                    IRGen irGen{ vslContext, *module };
                    ast->accept(irGen);
                    CodeGen codeGen{ vslContext, *module };
                    if (op.optimize)
                    {
                        codeGen.optimize();
                    }
                    out << *module << '\n';
                }
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
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> in =
        llvm::MemoryBuffer::getFileOrSTDIN(op.infile);
    if (std::error_code ec = in.getError())
    {
        llvm::errs() << "Error: could not open file '" << op.infile << "': " <<
            ec.message() << '\n';
        return 1;
    }
    VSLContext vslContext;
    VSLLexer lexer{ vslContext, in.get()->getBuffer().data() };
    VSLParser parser{ vslContext, lexer };
    auto ast = parser.parse();
    llvm::LLVMContext llvmContext;
    auto module = std::make_unique<llvm::Module>(op.infile, llvmContext);
    IRGen irGen{ vslContext, *module };
    ast->accept(irGen);
    if (vslContext.getErrorCount())
    {
        return 1;
    }
    CodeGen codeGen{ vslContext, *module };
    if (op.optimize)
    {
        codeGen.optimize();
    }
    std::error_code ec;
    llvm::raw_fd_ostream out{ op.outfile, ec, llvm::sys::fs::F_None };
    if (ec)
    {
        llvm::errs() << "Error: could not open file '" << op.outfile << "': " <<
            ec.message() << '\n';
        return 1;
    }
    codeGen.compile(out);
    if (vslContext.getErrorCount())
    {
        return 1;
    }
    return 0;
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
