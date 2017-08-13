#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "location.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <unordered_map>

class Token;

std::ostream& operator<<(std::ostream& os, const Token& token);

class Token
{
public:
    enum Kind
    {
        NUMBER,
        IDENTIFIER,
        KEYWORD_VAR,
        KEYWORD_LET,
        KEYWORD_FUNC,
        KEYWORD_RETURN,
        KEYWORD_IF,
        KEYWORD_ELSE,
        KEYWORD_INT,
        KEYWORD_VOID,
        OP_PLUS,
        OP_MINUS,
        OP_STAR,
        OP_SLASH,
        OP_PERCENT,
        OP_ASSIGN,
        OP_EQUALS,
        OP_GREATER,
        OP_GREATER_EQUAL,
        OP_LESS,
        OP_LESS_EQUAL,
        SYMBOL_COLON,
        SYMBOL_SEMICOLON,
        SYMBOL_COMMA,
        SYMBOL_ARROW,
        SYMBOL_LPAREN,
        SYMBOL_RPAREN,
        SYMBOL_LBRACE,
        SYMBOL_RBRACE,
        SYMBOL_EOF
    };
    Token(Kind kind, Location location);
    virtual ~Token() = 0;
    static const char* kindToString(Kind kind);
    virtual std::string toString() const = 0;
    Kind kind;
    Location location;
};

class DefaultToken : public Token
{
public:
    DefaultToken(Kind kind, Location location);
    virtual ~DefaultToken() = default;
    virtual std::string toString() const override;
};

class NameToken : public Token
{
public:
    NameToken(std::string name, Location location);
    virtual ~NameToken() override = default;
    virtual std::string toString() const override;
    std::string name;

private:
    static Token::Kind evaluateName(const std::string& name);
    static const std::unordered_map<std::string, Token::Kind> keywords;
};

class NumberToken : public Token
{
public:
    NumberToken(long value, Location location);
    virtual ~NumberToken() override = default;
    virtual std::string toString() const override;
    long value;
};

#endif // TOKEN_HPP
