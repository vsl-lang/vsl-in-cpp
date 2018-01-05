#include "ast/opKind.hpp"

const char* unaryKindSymbol(UnaryKind k)
{
    switch (k)
    {
        /** @cond */
#define UNARY(kind, name) case UnaryKind::kind: return name;
        /** @endcond */
    default:
        // falthrough to unknown
#include "ast/opKind.def"
    }
}

const char* binaryKindSymbol(BinaryKind k)
{
    switch (k)
    {
        /** @cond */
#define BINARY(kind, name) case BinaryKind::kind: return name;
        /** @endcond */
    default:
        // fallthrough to unknown
#include "ast/opKind.def"
    }
}

UnaryKind tokenKindToUnary(TokenKind k)
{
    switch (k)
    {
        /** @cond */
#define UNARY(kind, name) case TokenKind::kind: return UnaryKind::kind;
        /** @endcond */
    default:
        // fallthrough to unknown
#include "ast/opKind.def"
    }
}

BinaryKind tokenKindToBinary(TokenKind k)
{
    switch (k)
    {
        /** @cond */
#define BINARY(kind, name) case TokenKind::kind: return BinaryKind::kind;
        /** @endcond */
    default:
        // fallthrough to unknown
#include "ast/opKind.def"
    }
}
