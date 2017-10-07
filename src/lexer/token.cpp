#include "lexer/token.hpp"
#include <utility>

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    return os << token.toString();
}

Token::Token(Kind kind, Location location)
    : kind{ kind }, location{ location }
{
}

Token::~Token()
{
}

const char* Token::kindToString(Kind kind)
{
    switch (kind)
    {
    case Token::NUMBER:
        return "number";
    case Token::IDENTIFIER:
        return "identifier";
    case Token::KEYWORD_VAR:
        return "'var'";
    case Token::KEYWORD_LET:
        return "'let'";
    case Token::KEYWORD_FUNC:
        return "'func'";
    case Token::KEYWORD_RETURN:
        return "'return'";
    case Token::KEYWORD_IF:
        return "'if'";
    case Token::KEYWORD_ELSE:
        return "'else'";
    case Token::KEYWORD_INT:
        return "'Int'";
    case Token::KEYWORD_VOID:
        return "'Void'";
    case Token::OP_PLUS:
        return "'+'";
    case Token::OP_MINUS:
        return "'-'";
    case Token::OP_STAR:
        return "'*'";
    case Token::OP_SLASH:
        return "'/'";
    case Token::OP_PERCENT:
        return "'%'";
    case Token::OP_ASSIGN:
        return "'='";
    case Token::OP_EQUALS:
        return "'=='";
    case Token::OP_GREATER:
        return "'>'";
    case Token::OP_GREATER_EQUAL:
        return "'>='";
    case Token::OP_LESS:
        return "'<'";
    case Token::OP_LESS_EQUAL:
        return "'<='";
    case Token::SYMBOL_COLON:
        return "':'";
    case Token::SYMBOL_SEMICOLON:
        return "';'";
    case Token::SYMBOL_COMMA:
        return "','";
    case Token::SYMBOL_ARROW:
        return "'->'";
    case Token::SYMBOL_LPAREN:
        return "'('";
    case Token::SYMBOL_RPAREN:
        return "')'";
    case Token::SYMBOL_LBRACE:
        return "'{'";
    case Token::SYMBOL_RBRACE:
        return "'}'";
    case Token::SYMBOL_EOF:
        return "eof";
    default:
        return "invalid";
    }
}

DefaultToken::DefaultToken(Kind kind, Location location)
    : Token{ kind, location }
{
}

std::string DefaultToken::toString() const
{
    return kindToString(kind);
}

NameToken::NameToken(std::string name, Location location)
    : Token{ evaluateName(name), location }, name{ std::move(name) }
{
}

std::string NameToken::toString() const
{
    std::string s;
    switch (kind)
    {
    case Token::KEYWORD_VAR:
    case Token::KEYWORD_LET:
    case Token::KEYWORD_FUNC:
    case Token::KEYWORD_RETURN:
    case Token::KEYWORD_IF:
    case Token::KEYWORD_ELSE:
    case Token::KEYWORD_INT:
    case Token::KEYWORD_VOID:
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

Token::Kind NameToken::evaluateName(const std::string& name)
{
    auto it = keywords.find(name);
    if (it != keywords.end())
    {
        return it->second;
    }
    return Token::IDENTIFIER;
}

const std::unordered_map<std::string, Token::Kind> NameToken::keywords
{
    { "var", Token::KEYWORD_VAR },
    { "let", Token::KEYWORD_LET },
    { "func", Token::KEYWORD_FUNC },
    { "return", Token::KEYWORD_RETURN },
    { "if", Token::KEYWORD_IF },
    { "else", Token::KEYWORD_ELSE },
    { "Int", Token::KEYWORD_INT },
    { "Void", Token::KEYWORD_VOID }
};

NumberToken::NumberToken(long value, Location location)
    : Token{ Token::NUMBER, location }, value { value }
{
}

std::string NumberToken::toString() const
{
    std::string s = "number ";
    s += std::to_string(value);
    return s;
}
