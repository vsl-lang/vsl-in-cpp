#include "token.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <unordered_map>
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
    case Token::NUMBER:
        return "number";
    case Token::IDENTIFIER:
        return "identifier";
    case Token::VAR:
        return "keyword var";
    case Token::LET:
        return "keyword let";
    case Token::FUNC:
        return "keyword func";
    case Token::RETURN:
        return "keyword return";
    case Token::IF:
        return "keyword if";
    case Token::ELSE:
        return "keyword else";
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
    case Token::VAR:
    case Token::LET:
    case Token::FUNC:
    case Token::RETURN:
    case Token::IF:
    case Token::ELSE:
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
    auto it = keywords.find(name);
    if (it != keywords.end())
    {
        return it->second;
    }
    return Token::IDENTIFIER;
}

const std::unordered_map<std::string, Token::Type> NameToken::keywords
{
    { "var", Token::VAR },
    { "let", Token::LET },
    { "func", Token::FUNC },
    { "return", Token::RETURN },
    { "if", Token::IF },
    { "else", Token::ELSE }
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
