#include "lexer.hpp"
#include "token.hpp"
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <utility>

Lexer::Lexer(const char* src)
    : src{ src }, pos{ src }
{
}

std::unique_ptr<Token> Lexer::next()
{
    for (; *pos != '\0'; ++pos)
    {
        switch (*pos)
        {
        case '+':
            return lexToken(Token::PLUS);
        case '-':
            if (lookAhead(1) == '>')
            {
                ++pos;
                return lexToken(Token::ARROW);
            }
            return lexToken(Token::MINUS);
        case '*':
            return lexToken(Token::STAR);
        case '/':
            switch (lookAhead(1))
            {
            case '/':
                lexLineComment();
                break;
            case '*':
                lexBlockComment();
                break;
            default:
                return lexToken(Token::SLASH);
            }
            break;
        case '%':
            return lexToken(Token::PERCENT);
        case '=':
            if (lookAhead(1) == '=')
            {
                ++pos;
                return lexToken(Token::EQUALS);
            }
            return lexToken(Token::ASSIGN);
        case '>':
            if (lookAhead(1) == '=')
            {
                ++pos;
                return lexToken(Token::GREATER_EQUAL);
            }
            return lexToken(Token::GREATER);
        case '<':
            if (lookAhead(1) == '=')
            {
                ++pos;
                return lexToken(Token::LESS_EQUAL);
            }
            return lexToken(Token::LESS);
        case ':':
            return lexToken(Token::COLON);
        case ';':
            return lexToken(Token::SEMICOLON);
        case ',':
            return lexToken(Token::COMMA);
        case '(':
            return lexToken(Token::LPAREN);
        case ')':
            return lexToken(Token::RPAREN);
        case '{':
            return lexToken(Token::LBRACE);
        case '}':
            return lexToken(Token::RBRACE);
        default:
            if (isalpha(*pos))
            {
                return lexName();
            }
            if (isdigit(*pos))
            {
                return lexNumber();
            }
        }
    }
    return std::make_unique<Token>(Token::END, pos - src);
}

bool Lexer::empty() const
{
    return *pos == '\0';
}

char Lexer::lookAhead(size_t k) const
{
    char c;
    for (size_t i = 0; i <= k; ++i)
    {
        c = pos[i];
        if (c == '\0')
        {
            break;
        }
    }
    return c;
}

std::unique_ptr<Token> Lexer::lexToken(Token::Type type)
{
    return std::make_unique<Token>(type, pos++ - src);
}

std::unique_ptr<NameToken> Lexer::lexName()
{
    std::string id;
    size_t savedPos = pos - src;
    do
    {
        id += *pos++;
    }
    while (isalnum(*pos));
    return std::make_unique<NameToken>(std::move(id), savedPos);
}

std::unique_ptr<NumberToken> Lexer::lexNumber()
{
    size_t savedPos = pos - src;
    char* newPos;
    long value = std::strtol(pos, &newPos, 0);
    pos = newPos;
    return std::make_unique<NumberToken>(value, savedPos);
}

void Lexer::lexLineComment()
{
    do
    {
        ++pos;
    }
    while (pos[1] != '\n' && pos[1] != '\0');
}

void Lexer::lexBlockComment()
{
    ++pos;
    do
    {
        ++pos;
    }
    while (*pos != '\0' && *pos != '*' && pos[1] != '/');
    ++pos;
}
