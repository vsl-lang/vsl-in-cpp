#include "codegen.hpp"
#include "llvmgen.hpp"
#include "node.hpp"
#include "token.hpp"
#include "vsllexer.hpp"
#include "vslparser.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Flags
{
    Flags()
        : h{ false }, l{ false }, p{ false }, g{ false }, optLevel{ 0 },
        infile{ nullptr }, outfile{ "a.out" }
    {
    }
    bool h, l, p, g;
    int optLevel;
    const char* infile;
    const char* outfile;
};

void help()
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
}

void lex()
{
    std::string input;
    while (std::cout.good() && std::cin.good())
    {
        std::cout << "> ";
        std::getline(std::cin, input);
        VSLLexer lexer{ input.c_str() };
        std::unique_ptr<Token> token;
        do
        {
            token = lexer.nextToken();
            std::cerr << *token << '\n';
        }
        while (token->kind != Token::SYMBOL_EOF);
    }
}

void parse()
{
    std::string input;
    while (std::cout.good() && std::cin.good())
    {
        std::cout << "> ";
        std::getline(std::cin, input);
        VSLLexer lexer{ input.c_str() };
        VSLParser parser{ lexer };
        std::cerr << parser.parse()->toString() << '\n';
    }
}

void generate(int optLevel)
{
    std::string input;
    while (std::cout.good() && std::cin.good())
    {
        std::cout << "> ";
        std::getline(std::cin, input);
        VSLLexer lexer{ input.c_str() };
        VSLParser parser{ lexer };
        auto ast = parser.parse();
        llvm::LLVMContext context;
        auto module = std::make_unique<llvm::Module>("file", context);
        LLVMGen llvmGen{ *module };
        ast->accept(llvmGen);
        CodeGen codeGen{ *module };
        codeGen.optimize(optLevel);
        std::cerr << llvmGen.getIR() << '\n';
    }
}

void compile(std::string input, llvm::raw_pwrite_stream& output, int optLevel)
{
    VSLLexer lexer{ input.c_str() };
    VSLParser parser{ lexer };
    auto ast = parser.parse();
    llvm::LLVMContext context;
    auto module = std::make_unique<llvm::Module>("file", context);
    LLVMGen llvmGen{ *module };
    ast->accept(llvmGen);
    CodeGen codeGen{ *module };
    codeGen.optimize(optLevel);
    codeGen.compile(output);
}

int main(int argc, char** argv)
{
    Flags flags;
    for (int i = 1; i < argc; ++i)
    {
        const char* arg = argv[i];
        if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
        {
            flags.h = true;
        }
        else if (!strcmp(arg, "-l"))
        {
            flags.l = true;
        }
        else if (!strcmp(arg, "-p"))
        {
            flags.p = true;
        }
        else if (!strcmp(arg, "-g"))
        {
            flags.g = true;
        }
        else if (!strcmp(arg, "-o"))
        {
            if (i + 1 < argc)
            {
                flags.outfile = argv[++i];
            }
            else
            {
                std::cerr << "Error: no output file given\n";
            }
        }
        else if (!strncmp(arg, "-O", 2))
        {
            if (arg[2] == '\0')
            {
                std::cerr << "Error: No optimization level specified.\n";
            }
            else
            {
                int optLevel = arg[2] - '0';
                if (optLevel != 0 && optLevel != 1)
                {
                    std::cerr << "Error: Unknown optimization level '"
                        << &arg[2] << "', assuming 0\n";
                    flags.optLevel = 0;
                }
                else
                {
                    flags.optLevel = optLevel;
                }
            }
        }
        // any other unknown flag, except "-" which means stdin
        else if (arg[0] == '-' && arg[1] != '\0')
        {
            std::cerr << "Error: unknown flag '" << arg << "'\n";
        }
        else if (flags.infile != nullptr)
        {
            std::cerr <<
                "Error: VSL currently doesn't support multiple input files\n";
        }
        else
        {
            flags.infile = arg;
        }
    }
    if (flags.h)
    {
        help();
    }
    else if (flags.l)
    {
        lex();
    }
    else if (flags.p)
    {
        parse();
    }
    else if (flags.g)
    {
        generate(flags.optLevel);
    }
    else if (flags.infile != nullptr)
    {
        std::ifstream f{ flags.infile };
        std::string input{ std::istreambuf_iterator<char>{ f },
            std::istreambuf_iterator<char>{} };
        // open the file to write the output to
        std::error_code ec;
        llvm::raw_fd_ostream output{ flags.outfile, ec, llvm::sys::fs::F_None };
        if (ec)
        {
            std::cerr << "Error: could not open file " << flags.outfile <<
                " for writing: " << ec.message() << '\n';
            return 1;
        }
        compile(input, output, flags.optLevel);
    }
    else
    {
        std::cerr << "Error: no input\n";
        return 1;
    }
    return 0;
}
