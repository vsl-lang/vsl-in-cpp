#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
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
        Lexer lexer{ input.c_str() };
        std::unique_ptr<Token> token;
        do
        {
            token = lexer.next();
            std::cerr << *token << '\n';
        }
        while (token->getType() != Token::END);
    }
}

void parse()
{
    std::string input;
    while (std::cout.good() && std::cin.good())
    {
        std::cout << "> ";
        std::getline(std::cin, input);
        Lexer lexer{ input.c_str() };
        std::vector<std::unique_ptr<Token>> tokens;
        do
        {
            tokens.emplace_back(lexer.next());
        }
        while (!lexer.empty());
        tokens.emplace_back(lexer.next());
        Parser parser{ std::move(tokens) };
        std::cerr << parser.parse()->toString() << '\n';
    }
}

void help()
{
    std::cout <<
        "Usage: vsl [options]\n"
        "Options:\n"
        "  -h --help Display this information.\n"
        "  -l        Start the lexer REPL.\n"
        "  -p        Start the parser REPL.\n";
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
