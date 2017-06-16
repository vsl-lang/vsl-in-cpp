#include "token.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <unordered_set>
#include <utility>

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    return os << token.toString();
}

Token::Token(Type type, size_t pos)
    : type{ type }, pos{ pos }
{
}

Token::~Token()
{
}

const char* Token::typeToString(Token::Type type)
{
    switch (type)
    {
    case Token::IDENTIFIER:
        return "identifier";
    case Token::KEYWORD:
        return "keyword";
    case Token::NUMBER:
        return "number";
    case Token::PLUS:
        return "operator +";
    case Token::MINUS:
        return "operator -";
    case Token::STAR:
        return "operator *";
    case Token::SLASH:
        return "operator /";
    case Token::PERCENT:
        return "operator %";
    case Token::ASSIGN:
        return "operator =";
    case Token::EQUALS:
        return "operator ==";
    case Token::GREATER:
        return "operator >";
    case Token::GREATER_EQUAL:
        return "operator >=";
    case Token::LESS:
        return "operator <";
    case Token::LESS_EQUAL:
        return "operator <=";
    case Token::COLON:
        return "symbol :";
    case Token::SEMICOLON:
        return "symbol ;";
    case Token::COMMA:
        return "symbol ,";
    case Token::ARROW:
        return "symbol ->";
    case Token::LPAREN:
        return "symbol (";
    case Token::RPAREN:
        return "symbol )";
    case Token::LBRACE:
        return "symbol {";
    case Token::RBRACE:
        return "symbol }";
    case Token::END:
        return "end";
    default:
        return "invalid";
    }
}

std::string Token::toString() const
{
    return typeToString(type);
}

Token::Type Token::getType() const
{
    return type;
}

size_t Token::getPos() const
{
    return pos;
}

NameToken::NameToken(std::string name, size_t pos)
    : Token{ evaluateName(name), pos }, name{ std::move(name) }
{
}

NameToken::~NameToken()
{
}

std::string NameToken::toString() const
{
    std::string s;
    switch (getType())
    {
    case Token::KEYWORD:
        s = "keyword ";
        break;
    case Token::IDENTIFIER:
        s = "identifier ";
        break;
    default:
        s = "invalidName "; // should never happen
    }
    s += name;
    return s;
}

const std::string& NameToken::getName() const
{
    return name;
}

Token::Type NameToken::evaluateName(const std::string& name)
{
    if (keywords.find(name) != keywords.end())
    {
        return Token::KEYWORD;
    }
    return Token::IDENTIFIER;
}

const std::unordered_set<std::string> NameToken::keywords
{
    "var", "let", "func", "return", "if", "else"
};

NumberToken::NumberToken(long value, size_t pos)
    : Token{ Token::NUMBER, pos }, value { value }
{
}

NumberToken::~NumberToken()
{
}

std::string NumberToken::toString() const
{
    std::string s = "number ";
    s += std::to_string(value);
    return s;
}

long NumberToken::getValue() const
{
    return value;
}
