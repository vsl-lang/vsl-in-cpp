#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <cstddef>
#include <ostream>
#include <string>
#include <unordered_set>

class Token;

std::ostream& operator<<(std::ostream& os, const Token& token);

class Token
{
public:
    enum Type
    {
        IDENTIFIER,
        KEYWORD,
        NUMBER,
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
    Token(Type type, size_t pos);
    virtual ~Token();
    static const char* typeToString(Type type);
    virtual std::string toString() const;
    Type getType() const;
    size_t getPos() const;

private:
    Type type;
    size_t pos;
};

class NameToken : public Token
{
public:
    NameToken(std::string name, size_t pos);
    virtual ~NameToken() override;
    virtual std::string toString() const override;
    const std::string& getName() const;

private:
    static Token::Type evaluateName(const std::string& name);
    static const std::unordered_set<std::string> keywords;
    std::string name;
};

class NumberToken : public Token
{
public:
    NumberToken(long value, size_t pos);
    virtual ~NumberToken() override;
    virtual std::string toString() const override;
    long getValue() const;

private:
    long value;
};

#endif // TOKEN_HPP
