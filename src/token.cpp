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
        return "'var'";
    case Token::LET:
        return "'let'";
    case Token::FUNC:
        return "'func'";
    case Token::RETURN:
        return "'return'";
    case Token::IF:
        return "'if'";
    case Token::ELSE:
        return "'else'";
    case Token::PLUS:
        return "'+'";
    case Token::MINUS:
        return "'-'";
    case Token::STAR:
        return "'*'";
    case Token::SLASH:
        return "'/'";
    case Token::PERCENT:
        return "'%'";
    case Token::ASSIGN:
        return "'='";
    case Token::EQUALS:
        return "'=='";
    case Token::GREATER:
        return "'>'";
    case Token::GREATER_EQUAL:
        return "'>='";
    case Token::LESS:
        return "'<'";
    case Token::LESS_EQUAL:
        return "'<='";
    case Token::COLON:
        return "':'";
    case Token::SEMICOLON:
        return "';'";
    case Token::COMMA:
        return "','";
    case Token::ARROW:
        return "'->'";
    case Token::LPAREN:
        return "'('";
    case Token::RPAREN:
        return "')'";
    case Token::LBRACE:
        return "'{'";
    case Token::RBRACE:
        return "'}'";
    case Token::END:
        return "eof";
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
