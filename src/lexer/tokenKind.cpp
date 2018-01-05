#include "lexer/tokenKind.hpp"
#include "llvm/ADT/StringMap.h"

/** Internal map of all keywords. */
static const llvm::StringMap<TokenKind> keywords
{
    /** @cond */
#define KEYWORD(kind, name) { name, TokenKind::KW_ ## kind },
    /** @endcond */
#include "lexer/tokenKind.def"
};

const char* tokenKindName(TokenKind k)
{
    switch (k)
    {
        /** @cond */
#define TOKEN(kind, name) case TokenKind::kind: return name;
#define KEYWORD(kind, name) TOKEN(KW_ ## kind, "'" name "'")
        /** @endcond */
    default:
        // fallthrough to unknown
#include "lexer/tokenKind.def"
    }
}

const char* tokenKindDebugName(TokenKind k)
{
    switch (k)
    {
        /** @cond */
#define TOKEN(kind, name) case TokenKind::kind: return #kind;
        /** @endcond */
    default:
        // fallthrough to unknown
#include "lexer/tokenKind.def"
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
