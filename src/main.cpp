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

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-l") == 0)
        {
            lex();
            return 0;
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            parse();
            return 0;
        }
        else if (strcmp(argv[i], "-g") == 0)
        {
            generate();
            return 0;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            help();
            return 0;
        }
        else
        {
            std::cerr << "Error: unknown argument '" << argv[i] << "'\n";
        }
    }
    std::cerr << "Error: no input\n";
    return 0;
}
