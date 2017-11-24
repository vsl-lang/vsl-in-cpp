#include "irgen/vslContext.hpp"
#include "lexer/vsllexer.hpp"
#include "parser/vslparser.hpp"
#include "gtest/gtest.h"

#define valid(src) EXPECT_TRUE(parse(src))
#define invalid(src) EXPECT_FALSE(parse(src))

// returns true if the syntax is valid, false otherwise
static bool parse(const char* src)
{
    VSLContext vslContext{ llvm::nulls() };
    VSLLexer lexer{ vslContext, src };
    VSLParser parser{ vslContext, lexer };
    parser.parse();
    return !vslContext.getErrorCount();
}

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

TEST(ParserTest, HandlesNumbers)
{
    valid("1337;");
    invalid("999999999999999999999999999;");
}

TEST(ParserTest, HandlesParenthesizedExprs)
{
    valid("(x + y) / 2;");
    invalid("(x + 1;");
    invalid("x + 1);");
}
