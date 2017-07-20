#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"
#include "location.hpp"
#include <cstddef>
#include <iostream>
#include <memory>

class Lexer
{
public:
    Lexer(const char* src, std::ostream& errors = std::cerr);
    std::unique_ptr<Token> nextToken();
    bool empty() const;

private:
    char current() const;
    char next();
    char peek(size_t i = 1) const;
    std::unique_ptr<Token> lexToken(Token::Kind kind);
    std::unique_ptr<NameToken> lexName();
    std::unique_ptr<NumberToken> lexNumber();
    void lexLineComment();
    void lexBlockComment();
    const char* src;
    Location location;
    std::ostream& errors;
};

#endif // LEXER_HPP
