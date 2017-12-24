#include "lexer/vsllexer.hpp"
#include <cctype>

VSLLexer::VSLLexer(Diag& diag, const char* src)
    : Lexer{ diag }, text{ src, 1 }, location{ 1, 1 }
{
}

Token VSLLexer::nextToken()
{
    while (current() != '\0')
    {
        switch (current())
        {
        case '+':
            return createToken(TokenKind::PLUS);
        case '-':
            if (peek() == '>')
            {
                next();
                return createToken(TokenKind::ARROW);
            }
            return createToken(TokenKind::MINUS);
        case '*':
            return createToken(TokenKind::STAR);
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
                return createToken(TokenKind::SLASH);
            }
            break;
        case '%':
            return createToken(TokenKind::PERCENT);
        case '=':
            if (peek() == '=')
            {
                next();
                return createToken(TokenKind::EQUAL);
            }
            return createToken(TokenKind::ASSIGN);
        case '>':
            if (peek() == '=')
            {
                next();
                return createToken(TokenKind::GREATER_EQUAL);
            }
            return createToken(TokenKind::GREATER);
        case '<':
            if (peek() == '=')
            {
                next();
                return createToken(TokenKind::LESS_EQUAL);
            }
            return createToken(TokenKind::LESS);
        case '(':
            return createToken(TokenKind::LPAREN);
        case ')':
            return createToken(TokenKind::RPAREN);
        case '{':
            return createToken(TokenKind::LBRACE);
        case '}':
            return createToken(TokenKind::RBRACE);
        case ',':
            return createToken(TokenKind::COMMA);
        case ':':
            return createToken(TokenKind::COLON);
        case ';':
            return createToken(TokenKind::SEMICOLON);
        case '\n':
            break;
        case '!':
            if (peek() == '=')
            {
                next();
                return createToken(TokenKind::NOT_EQUAL);
            }
            // fallthrough
        default:
            if (isalpha(current()))
            {
                return lexIdentOrKeyword();
            }
            if (isdigit(current()))
            {
                return lexNumber();
            }
            if (!isspace(current()))
            {
                diag.print<Diag::UNKNOWN_SYMBOL>(location, current());
            }
        }
        resetBuffer();
    }
    return createToken(TokenKind::END);
}

bool VSLLexer::empty() const
{
    return current() == '\0';
}

char VSLLexer::current() const
{
    return text.back();
}

char VSLLexer::next()
{
    text = { text.begin(), text.size() + 1 };
    return text[0];
}

void VSLLexer::resetBuffer()
{
    // handle newlines
    if (current() == '\n')
    {
        ++location.line;
        location.col = 0;
    }
    else
    {
        location.col += text.size();
    }
    text = { text.end(), 1 };
}

char VSLLexer::peek() const
{
    char c = current();
    if (c == '\0')
    {
        return c;
    }
    return *text.end();
}

Token VSLLexer::createToken(TokenKind kind)
{
    Token t{ kind, text, location };
    resetBuffer();
    return t;
}

Token VSLLexer::lexIdentOrKeyword()
{
    while (isalnum(peek()))
    {
        next();
    }
    return createToken(getKeywordKind(text));
}

Token VSLLexer::lexNumber()
{
    while (isdigit(peek()))
    {
        next();
    }
    return createToken(TokenKind::NUMBER);
}

void VSLLexer::lexLineComment()
{
    do
    {
        next();
    }
    while (peek() != '\n' && peek() != '\0');
    resetBuffer();
}

void VSLLexer::lexBlockComment()
{
    resetBuffer();
    do
    {
        resetBuffer();
    }
    while (current() != '\0' && current() != '*' && peek() != '/');
    next();
}
