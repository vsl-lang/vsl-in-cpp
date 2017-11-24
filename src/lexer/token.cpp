#include "lexer/token.hpp"
#include <utility>

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const Token& token)
{
    return os << tokenKindDebugName(token.kind) << " '" << token.text.str() <<
        '\'';
}

Token::Token()
    : kind{ TokenKind::UNKNOWN }
{
}

Token::Token(TokenKind kind, llvm::StringRef text, Location location)
    : kind{ kind }, text{ text }, location{ location }
{
}
