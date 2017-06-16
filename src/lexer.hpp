#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"
#include <cstddef>
#include <memory>

class Lexer
{
public:
    Lexer(const char* src);
    std::unique_ptr<Token> next();
    bool empty() const;

private:
    char lookAhead(size_t k) const;
    std::unique_ptr<Token> lexToken(Token::Type type);
    std::unique_ptr<NameToken> lexName();
    std::unique_ptr<NumberToken> lexNumber();
    void lexLineComment();
    void lexBlockComment();
    const char* src;
    const char* pos;
};

#endif // LEXER_HPP
