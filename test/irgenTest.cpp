#include "diag/diag.hpp"
#include "irgen/irgen.hpp"
#include "irgen/vslContext.hpp"
#include "lexer/vsllexer.hpp"
#include "parser/vslparser.hpp"
#include "gtest/gtest.h"

#define valid(src) EXPECT_TRUE(validate(src))
#define invalid(src) EXPECT_FALSE(validate(src))

// returns true if semantically valid, false otherwise
static bool validate(const char* src)
{
    VSLContext vslContext;
    Diag diag{ llvm::nulls() };
    VSLLexer lexer{ diag, src };
    VSLParser parser{ vslContext, lexer };
    auto* program = parser.parse();
    llvm::LLVMContext llvmContext;
    auto module = std::make_unique<llvm::Module>("test", llvmContext);
    IRGen irgen{ vslContext, diag, *module };
    program->accept(irgen);
    return !diag.getNumErrors();
}

TEST(IRGenTest, Functions)
{
    valid("func f() -> Void {}");
    valid("func f(x: Int) -> Void { return; }");
    valid("func f(x: Int) -> Int { return x + 1; }");
    // can't have void parameters
    invalid("func f(x: Void) -> Void { return x; }");
    // can't return a void expression
    invalid("func f() -> Void { return f(); }");
}

TEST(IRGenTest, IfStatements)
{
    // else case is optional
    valid("func f(x: Int) -> Int { if (x % 2 == 0) return 1337; return x; }");
    // if/else can be nested
    valid("func f(x: Int) -> Int "
        "{ "
            "if (x > 0) "
                "if (x > 1337) "
                    "x = 5; "
                "else "
                    "return 1; "
            "else "
                "return 2; "
            "return x; "
        "}");
    // if/else can be chained
    valid("func f(x: Int) -> Int { if (x == 0) return 0; "
        "else if (x == 1) return 1; else return x; }");
    valid("func fibonacci(x: Int) -> Int "
        "{ "
            "if (x <= 0) return 0; "
            "else if (x == 1) return 1; "
            "else return fibonacci(x: x - 1) + fibonacci(x: x - 2); "
        "}");
}

TEST(IRGenTest, Variables)
{
    valid("func f(x: Int) -> Int { let y: Int = x * 2; y = y / x; return y; }");
}
