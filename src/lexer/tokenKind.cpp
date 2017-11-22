#include "lexer/tokenKind.hpp"
#include "llvm/ADT/StringMap.h"

static const llvm::StringMap<TokenKind> keywords
{
#define KEYWORD(X, Y) { Y, TokenKind::KW_ ## X },
#include "lexer/tokenKind.def"
};

const char* tokenKindName(TokenKind k)
{
    switch (k)
    {
#define TOKEN(X, Y) case TokenKind::X: return Y;
#define KEYWORD(X, Y) TOKEN(KW_ ## X, "'" Y "'")
#include "lexer/tokenKind.def"
    default:
        return "unknown";
    }
}

const char* tokenKindDebugName(TokenKind k)
{
    switch (k)
    {
#define TOKEN(X, Y) case TokenKind::X: return #X;
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
