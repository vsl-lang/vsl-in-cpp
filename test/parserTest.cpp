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
    valid("public func f(x: Int) -> Void {;}");
    valid("public func f(x: Bool, y: Int) -> Bool external(g);");
    // invalid usave of access specifiers
    invalid("public public func f() -> Void {}");
    invalid("func f() -> Void {}");
    // void parameters are rejected
    invalid("public func f(x: Void, y: Int) -> Int { return 0; }");
    // can't define a function within a function
    invalid("public func f() -> Void { private func g() -> Void {} }");
}

TEST(ParserTest, EmptyStmts)
{
    valid("public func f() -> Void { ; }");
    valid("public func f() -> Void { ;;;;; }");
}

TEST(ParserTest, Blocks)
{
    valid("public func f() -> Void { { hi; } }");
    // unbalanced braces
    invalid("public func f() -> Void { { hi; }");
    invalid("public func f() -> Void { hi; } }");
}

TEST(ParserTest, Ifs)
{
    valid("public func f() -> Void { if (x == 1) {;} }");
    invalid("public func f() -> Void { if x == 1 {;} }");
    // parser only does syntax checking so this should still be fine
    valid("public func f() -> Void { if (x == 1) { return 3; } else return x; "
        "}");
}

TEST(ParserTest, Variables)
{
    valid("public func f() -> Void { let x: Int = 1337; }");
    valid("public func f() -> Void { var y: Void = x; }");
}

TEST(ParserTest, Returns)
{
    valid("public func f() -> Void { return; }");
    valid("public func f() -> Void { return x; }");
}

TEST(ParserTest, Expressions)
{
    valid("public func f() -> Void { -x-1; }");
    invalid("public func f() -> Void { x*; }");
    // needs a semicolon at the end
    invalid("public func f() -> Void { x }");
    // some weird expression
    valid("public func f() -> Void { x(y: z % 2) - -z * 2 + 9 / 2; }");
}

TEST(ParserTest, Numbers)
{
    valid("public func f() -> Int { return 1337; }");
    // emits a warning so this should be fine
    valid("public func f() -> Int { return 999999999999999999999999999; }");
}

TEST(ParserTest, Parentheses)
{
    valid("public func f() -> Void { (x + y) / 2; }");
    invalid("public func f() -> Void { (x + 1; }");
    invalid("public func f() -> Void { x + 1); }");
}
