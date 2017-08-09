#ifndef VSLLEXER_HPP
#define VSLLEXER_HPP

#include "lexer.hpp"
#include "location.hpp"
#include "token.hpp"
#include <cstddef>
#include <iostream>
#include <memory>

class VSLLexer : public Lexer
{
public:
    VSLLexer(const char* src, std::ostream& errors = std::cerr);
    virtual ~VSLLexer() override = default;
    virtual std::unique_ptr<Token> nextToken() override;
    virtual bool empty() const override;

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

#endif // VSLLEXER_HPP
