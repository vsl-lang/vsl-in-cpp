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
    enum Type
    {
        // regular old terminals
        NUMBER,
        IDENTIFIER,
        // keywords
        VAR,
        LET,
        FUNC,
        RETURN,
        IF,
        ELSE,
        // operators
        PLUS,
        MINUS,
        STAR,
        SLASH,
        PERCENT,
        ASSIGN,
        EQUALS,
        GREATER,
        GREATER_EQUAL,
        LESS,
        LESS_EQUAL,
        // symbols
        COLON,
        SEMICOLON,
        COMMA,
        ARROW,
        LPAREN,
        RPAREN,
        LBRACE,
        RBRACE,
        END
    };
    Token(Type type, Location location);
    virtual ~Token();
    static const char* typeToString(Type type);
    virtual std::string toString() const;
    Type getType() const;
    Location getLocation() const;

private:
    Type type;
    Location location;
};

class NameToken : public Token
{
public:
    NameToken(std::string name, Location location);
    virtual ~NameToken() override;
    virtual std::string toString() const override;
    const std::string& getName() const;

private:
    static Token::Type evaluateName(const std::string& name);
    static const std::unordered_map<std::string, Token::Type> keywords;
    std::string name;
};

class NumberToken : public Token
{
public:
    NumberToken(long value, Location location);
    virtual ~NumberToken() override;
    virtual std::string toString() const override;
    long getValue() const;

private:
    long value;
};

#endif // TOKEN_HPP
