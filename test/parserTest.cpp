#include "vsllexer.hpp"
#include "vslparser.hpp"
#include "gtest/gtest.h"

#define valid(src) EXPECT_TRUE(parse(src))
#define invalid(src) EXPECT_FALSE(parse(src))

namespace
{
// returns true if there's a syntax error, false otherwise
bool parse(const char* src)
{
    VSLLexer lexer{ src };
    VSLParser parser{ lexer };
    parser.parse();
    return !parser.hasError();
}
} // namespace

TEST(ParserTest, HandlesEmptyStatement)
{
    valid(";");
    valid(";;;;;");
}

TEST(ParserTest, HandlesBlock)
{
    valid("{hi;}");
    invalid("{hi;");
}

TEST(ParserTest, HandlesConditional)
{
    valid("if (x == 1) {;}");
    invalid("if x == 1 {;}");
}

TEST(ParserTest, HandlesAssignment)
{
    valid("let x: Int = 1337;");
    valid("var y: Void = x;");
}

TEST(ParserTest, HandlesFunction)
{
    valid("func f(x: Int) -> Void {;}");
}

TEST(ParserTest, HandlesReturn)
{
    valid("return x;");
}

TEST(ParserTest, HandlesExpr)
{
    valid("-x-1;");
    invalid("x*;");
    // needs a semicolon at the end
    invalid("x");
    valid("x(y: z % 2) - -z * 2 + 9 / 2;");
}

TEST(ParserTest, HandlesParenthesizedExprs)
{
    valid("(x + y) / 2;");
    invalid("(x + 1;");
    invalid("x + 1);");
}
