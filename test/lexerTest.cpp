#include "lexer/token.hpp"
#include "lexer/vsllexer.hpp"
#include "gtest/gtest.h"

#define valid(src) EXPECT_TRUE(lex(src))
#define invalid(src) EXPECT_FALSE(lex(src))

namespace
{
// returns true if there's a lexical error, false otherwise
bool lex(const char* src)
{
    VSLLexer lexer{ src };
    while(!lexer.empty())
    {
        lexer.nextToken();
    }
    return !lexer.hasError();
}
} // namespace

TEST(LexerTest, NotEmptyOnInit)
{
    VSLLexer lexer{ "hi" };
    EXPECT_FALSE(lexer.empty());
}

TEST(LexerTest, HandlesNumbers)
{
    // small numberes are ok
    valid("1337");
    // but not big numbers like these
    invalid("999999999999999999999999999999999");
}
