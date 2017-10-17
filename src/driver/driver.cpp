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
        return repl([](const std::string& in, std::ostream& out)
            {
                VSLLexer lexer{ in.c_str(), out };
                std::unique_ptr<Token> token;
                do
                {
                    token = lexer.nextToken();
                    out << *token << '\n';
                }
                while (token->kind != Token::SYMBOL_EOF);
            });
    case OptionParser::REPL_PARSE:
        return repl([](const std::string& in, std::ostream& out)
            {
                VSLLexer lexer{ in.c_str(), out };
                VSLParser parser{ lexer, out };
                out << parser.parse()->toString() << '\n';
            });
    case OptionParser::REPL_GENERATE:
        return repl([&](const std::string& in, std::ostream& out)
            {
                VSLLexer lexer{ in.c_str(), out };
                VSLParser parser{ lexer, out };
                auto ast = parser.parse();
                llvm::LLVMContext context;
                auto module = std::make_unique<llvm::Module>("repl", context);
                IRGen irGen{ *module };
                ast->accept(irGen);
                CodeGen codeGen{ *module };
                if (op.optimize)
                {
                    codeGen.optimize();
                }
                std::cerr << irGen.getIR() << '\n';
            });
    }
    return 0;
}

int Driver::displayHelp()
{
    std::cout <<
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
        std::cerr << ec << '\n';
        return 1;
    }
    VSLLexer lexer{ in.get()->getBuffer().data() };
    VSLParser parser{ lexer };
    auto ast = parser.parse();
    llvm::LLVMContext context;
    auto module = std::make_unique<llvm::Module>(op.infile, context);
    IRGen irGen{ *module };
    ast->accept(irGen);
    if (lexer.hasError() || parser.hasError() || irGen.hasError())
    {
        return 1;
    }
    CodeGen codeGen{ *module };
    if (op.optimize)
    {
        codeGen.optimize();
    }
    std::error_code ec;
    llvm::raw_fd_ostream out{ op.outfile, ec, llvm::sys::fs::F_None };
    if (ec)
    {
        std::cerr << ec << '\n';
        return 1;
    }
    codeGen.compile(out);
    if (codeGen.hasError())
    {
        return 1;
    }
    return 0;
}

int Driver::repl(std::function<void(const std::string&, std::ostream&)>
    evaluator)
{
    std::string input;
    while (std::cout.good() && std::cin.good())
    {
        std::cout << "> ";
        std::getline(std::cin, input);
        evaluator(input, std::cerr);
        input.clear();
    }
    return 0;
}
