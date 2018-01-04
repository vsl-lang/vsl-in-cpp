#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "lexer/vsllexer.hpp"
#include "parser/vslparser.hpp"
#include "gtest/gtest.h"

#define valid(src) EXPECT_TRUE(parse(src))
#define invalid(src) EXPECT_FALSE(parse(src))

// returns true if the syntax is valid, false otherwise
static bool parse(const char* src)
{
    VSLContext vslContext;
    Diag diag{ llvm::nulls() };
    VSLLexer lexer{ diag, src };
    VSLParser parser{ vslContext, lexer };
    parser.parse();
    return !diag.getNumErrors();
}

TEST(ParserTest, OnlyGlobalsAllowed)
{
    invalid("let x: Int = 1;");
    invalid("if (true) d; else c;");
    invalid("return;");
}

TEST(ParserTest, Functions)
{
    valid("func f(x: Int) -> Void {;}");
    valid("func f(x: Bool, y: Int) -> Bool external(g);");
    // void parameters are rejected
    invalid("func f(x: Void, y: Int) -> Int { return 0; }");
    // can't define a function within a function
    invalid("func f() -> Void { func g() -> Void {} }");
}

TEST(ParserTest, EmptyStmts)
{
    valid("func f() -> Void { ; }");
    valid("func f() -> Void { ;;;;; }");
}

TEST(ParserTest, Blocks)
{
    valid("func f() -> Void { { hi; } }");
    // unbalanced braces
    invalid("func f() -> Void { { hi; }");
    invalid("func f() -> Void { hi; } }");
}

TEST(ParserTest, Ifs)
{
    valid("func f() -> Void { if (x == 1) {;} }");
    invalid("func f() -> Void { if x == 1 {;} }");
    // parser only does syntax checking so this should still be fine
    valid("func f() -> Void { if (x == 1) { return 3; } else return x; }");
}

TEST(ParserTest, Variables)
{
    valid("func f() -> Void { let x: Int = 1337; }");
    valid("func f() -> Void { var y: Void = x; }");
}

TEST(ParserTest, Returns)
{
    valid("func f() -> Void { return; }");
    valid("func f() -> Void { return x; }");
}

TEST(ParserTest, Expressions)
{
    valid("func f() -> Void { -x-1; }");
    invalid("func f() -> Void { x*; }");
    // needs a semicolon at the end
    invalid("func f() -> Void { x }");
    // some weird expression
    valid("func f() -> Void { x(y: z % 2) - -z * 2 + 9 / 2; }");
}

TEST(ParserTest, Numbers)
{
    valid("func f() -> Int { return 1337; }");
    // emits a warning so this should be fine
    valid("func f() -> Int { return 999999999999999999999999999; }");
}

TEST(ParserTest, Parentheses)
{
    valid("func f() -> Void { (x + y) / 2; }");
    invalid("func f() -> Void { (x + 1; }");
    invalid("func f() -> Void { x + 1); }");
}
