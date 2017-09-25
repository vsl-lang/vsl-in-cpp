#include "llvmgen.hpp"
#include "node.hpp"
#include "token.hpp"
#include "vsllexer.hpp"
#include "vslparser.hpp"
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Flags
{
    Flags()
        : h{ false }, l{ false }, p{ false }, g{ false }
    {
    }
    bool h, l, p, g;
};

void help()
{
    std::cout <<
        "Usage: vsl [options]\n"
        "Options:\n"
        "  -h --help Display this information.\n"
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

void generate()
{
    std::string input;
    while (std::cout.good() && std::cin.good())
    {
        std::cout << "> ";
        std::getline(std::cin, input);
        VSLLexer lexer{ input.c_str() };
        VSLParser parser{ lexer };
        LLVMGen gen;
        auto ast = parser.parse();
        ast->accept(gen);
        std::cerr << gen.getIR() << '\n';
    }
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
        else
        {
            std::cerr << "Error: unknown argument '" << arg << "'\n";
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
        generate();
    }
    else
    {
        std::cerr << "Error: no input\n";
        return 1;
    }
    return 0;
}
