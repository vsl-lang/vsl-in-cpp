#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"
#include <memory>

class Lexer
{
public:
    virtual ~Lexer() = 0;
    virtual std::unique_ptr<Token> nextToken() = 0;
    virtual bool empty() const = 0;
};

#endif // LEXER_HPP
