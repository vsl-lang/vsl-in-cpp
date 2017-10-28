#include "lexer/tokenKind.hpp"
#include "llvm/ADT/StringMap.h"

static const llvm::StringMap<TokenKind> keywords
{
#undef KEYWORD
#define KEYWORD(X, Y) { Y, TokenKind::KW_ ## X },
#include "lexer/tokenKind.def"
#undef KEYWORD
};

const char* getTokenKindName(TokenKind tk)
{
    switch (tk)
    {
#undef TOKEN
#define TOKEN(X) case TokenKind::X: return #X;
#include "lexer/tokenKind.def"
    default:
        return "UNKNOWN";
    }
}

TokenKind getKeywordKind(llvm::StringRef s)
{
    auto it = keywords.find(s);
    if (it != keywords.end())
    {
        // the keyword was found
        return (*it).getValue();
    }
    // the keyword was not found, assume identifier
    return TokenKind::IDENTIFIER;
}
