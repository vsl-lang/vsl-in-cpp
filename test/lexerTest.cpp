#include "diag/diag.hpp"
#include "lexer/token.hpp"
#include "lexer/vsllexer.hpp"
#include "gtest/gtest.h"

#define valid(src) EXPECT_TRUE(lex(src))
#define invalid(src) EXPECT_FALSE(lex(src))

// returns true if the tokens are valid, false otherwise
static bool lex(const char* src)
{
    Diag diag{ llvm::nulls() };
    VSLLexer lexer{ diag, src };
    while(!lexer.empty())
    {
        lexer.nextToken();
    }
    return !diag.getNumErrors();
}

TEST(LexerTest, NotEmptyOnInit)
{
    Diag diag{ llvm::nulls() };
    VSLLexer lexer{ diag, "hi" };
    EXPECT_FALSE(lexer.empty());
}

TEST(LexerTest, HandlesNumbers)
{
    // small numberes are ok
    valid("1337");
    // and so are big numbers like these
    valid("999999999999999999999999999999999");
}
