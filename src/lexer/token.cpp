#include "lexer/token.hpp"
#include <utility>

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const Token& token)
{
    return os << tokenKindDebugName(token.kind) << " '" << token.text.str() <<
        '\'' << " at " << token.location;
}

Token::Token()
    : kind{ TokenKind::UNKNOWN }
{
}

Token::Token(TokenKind kind, llvm::StringRef text, Location location)
    : kind{ kind }, text{ text }, location{ location }
{
}

TokenKind Token::getKind() const
{
    return kind;
}

bool Token::is(TokenKind k) const
{
    return kind == k;
}

bool Token::isNot(TokenKind k) const
{
    return kind != k;
}

const char* Token::getKindName() const
{
    return tokenKindName(kind);
}

llvm::StringRef Token::getText() const
{
    return text;
}

Location Token::getLoc() const
{
    return location;
}
