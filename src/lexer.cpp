#include "lexer.hpp"
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <utility>

Lexer::Lexer(const char* src, std::ostream& errors)
    : src{ src }, location{ src, 1, 1 }, errors{ errors }
{
}

std::unique_ptr<Token> Lexer::nextToken()
{
    for (; current() != '\0'; next())
    {
        switch (current())
        {
        case '\n':
            ++location.line;
            location.col = 0;
            break;
        case '+':
            return lexToken(Token::PLUS);
        case '-':
            if (peek() == '>')
            {
                next();
                return lexToken(Token::ARROW);
            }
            return lexToken(Token::MINUS);
        case '*':
            return lexToken(Token::STAR);
        case '/':
            switch (peek())
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
            if (peek() == '=')
            {
                next();
                return lexToken(Token::EQUALS);
            }
            return lexToken(Token::ASSIGN);
        case '>':
            if (peek() == '=')
            {
                next();
                return lexToken(Token::GREATER_EQUAL);
            }
            return lexToken(Token::GREATER);
        case '<':
            if (peek() == '=')
            {
                next();
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
            if (isalpha(current()))
            {
                return lexName();
            }
            if (isdigit(current()))
            {
                return lexNumber();
            }
            if (!isspace(current()))
            {
                errors << location << ": error: unknown symbol '" <<
                    current() << "'\n";
            }
        }
    }
    return std::make_unique<Token>(Token::END, location);
}

bool Lexer::empty() const
{
    return current() == '\0';
}

char Lexer::current() const
{
    return *location.pos;
}

char Lexer::next()
{
    ++location.col;
    return *++location.pos;
}

char Lexer::peek(size_t i) const
{
    char c;
    for (size_t j = 0; j <= i; ++j)
    {
        c = location.pos[j];
        if (c == '\0')
        {
            break;
        }
    }
    return c;
}

std::unique_ptr<Token> Lexer::lexToken(Token::Type type)
{
    Location savedLocation = location;
    next();
    return std::make_unique<Token>(type, savedLocation);
}

std::unique_ptr<NameToken> Lexer::lexName()
{
    Location savedLocation = location;
    std::string id;
    do
    {
        id += current();
        next();
    }
    while (isalnum(current()));
    return std::make_unique<NameToken>(std::move(id), savedLocation);
}

std::unique_ptr<NumberToken> Lexer::lexNumber()
{
    Location savedLocation = location;
    char* newPos;
    errno = 0;
    long value = std::strtol(location.pos, &newPos, 0);
    location.col += newPos - location.pos;
    location.pos = newPos;
    if (errno == ERANGE)
    {
        errors << location << ": warning: ";
        errors.write(savedLocation.pos, location.pos - savedLocation.pos);
        errors << " is out of range\n";
    }
    return std::make_unique<NumberToken>(value, savedLocation);
}

void Lexer::lexLineComment()
{
    do
    {
        next();
    }
    while (peek() != '\n' && peek() != '\0');
}

void Lexer::lexBlockComment()
{
    next();
    do
    {
        next();
    }
    while (current() != '\0' && current() != '*' && peek() != '/');
    next();
}
