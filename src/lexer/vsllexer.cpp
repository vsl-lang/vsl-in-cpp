#include "lexer/vsllexer.hpp"
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <utility>

VSLLexer::VSLLexer(const char* src, std::ostream& errors)
    : src{ src }, location{ src, 1, 1 }, errors{ errors }, errored{ false }
{
}

std::unique_ptr<Token> VSLLexer::nextToken()
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
            return lexDefaultToken(Token::OP_PLUS);
        case '-':
            if (peek() == '>')
            {
                next();
                return lexDefaultToken(Token::SYMBOL_ARROW);
            }
            return lexDefaultToken(Token::OP_MINUS);
        case '*':
            return lexDefaultToken(Token::OP_STAR);
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
                return lexDefaultToken(Token::OP_SLASH);
            }
            break;
        case '%':
            return lexDefaultToken(Token::OP_PERCENT);
        case '=':
            if (peek() == '=')
            {
                next();
                return lexDefaultToken(Token::OP_EQUALS);
            }
            return lexDefaultToken(Token::OP_ASSIGN);
        case '>':
            if (peek() == '=')
            {
                next();
                return lexDefaultToken(Token::OP_GREATER_EQUAL);
            }
            return lexDefaultToken(Token::OP_GREATER);
        case '<':
            if (peek() == '=')
            {
                next();
                return lexDefaultToken(Token::OP_LESS_EQUAL);
            }
            return lexDefaultToken(Token::OP_LESS);
        case ':':
            return lexDefaultToken(Token::SYMBOL_COLON);
        case ';':
            return lexDefaultToken(Token::SYMBOL_SEMICOLON);
        case ',':
            return lexDefaultToken(Token::SYMBOL_COMMA);
        case '(':
            return lexDefaultToken(Token::SYMBOL_LPAREN);
        case ')':
            return lexDefaultToken(Token::SYMBOL_RPAREN);
        case '{':
            return lexDefaultToken(Token::SYMBOL_LBRACE);
        case '}':
            return lexDefaultToken(Token::SYMBOL_RBRACE);
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
                errored = true;
            }
        }
    }
    return lexDefaultToken(Token::SYMBOL_EOF);
}

bool VSLLexer::empty() const
{
    return current() == '\0';
}

bool VSLLexer::hasError() const
{
    return errored;
}

char VSLLexer::current() const
{
    return *location.pos;
}

char VSLLexer::next()
{
    ++location.col;
    return *++location.pos;
}

char VSLLexer::peek(size_t i) const
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

std::unique_ptr<DefaultToken> VSLLexer::lexDefaultToken(Token::Kind kind)
{
    Location savedLocation = location;
    next();
    return std::make_unique<DefaultToken>(kind, savedLocation);
}

std::unique_ptr<NameToken> VSLLexer::lexName()
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

std::unique_ptr<NumberToken> VSLLexer::lexNumber()
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
        errored = true;
    }
    return std::make_unique<NumberToken>(value, savedLocation);
}

void VSLLexer::lexLineComment()
{
    do
    {
        next();
    }
    while (peek() != '\n' && peek() != '\0');
}

void VSLLexer::lexBlockComment()
{
    next();
    do
    {
        next();
    }
    while (current() != '\0' && current() != '*' && peek() != '/');
    next();
}
