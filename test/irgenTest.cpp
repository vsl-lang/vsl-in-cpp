#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "irgen/irgen.hpp"
#include "lexer/vsllexer.hpp"
#include "parser/vslparser.hpp"
#include "gtest/gtest.h"

#define valid(src) EXPECT_TRUE(validate(src))
#define invalid(src) EXPECT_FALSE(validate(src))

// returns true if semantically valid, false otherwise
static bool validate(const char* src)
{
    VSLContext vslCtx;
    Diag diag{ llvm::nulls() };
    VSLLexer lexer{ diag, src };
    VSLParser parser{ vslCtx, lexer };
    parser.parse();
    llvm::LLVMContext llvmContext;
    auto module = std::make_unique<llvm::Module>("test", llvmContext);
    IRGen irgen{ vslCtx, diag, *module };
    irgen.run();
    return !diag.getNumErrors();
}

TEST(IRGenTest, Functions)
{
    valid("public func f() -> Void {}");
    valid("public func f(x: Int) -> Void { return; }");
    valid("public func f(x: Int) -> Int { return x + 1; }");
    valid("public func f() -> Void { g(); } "
        "public func g() -> Void external(h);");
    invalid("public func f() -> Void { h(); } "
        "public func g() -> Void external(h);");
    // can't have void parameters
    invalid("public func f(x: Void) -> Void { return x; }");
    // can't return a void expression
    invalid("public func f() -> Void { return f(); }");
    // able to call a function ahead of its definition
    valid("public func f() -> Void { g(); } private func g() -> Void {}");
    // can't call non-functions
    invalid("public func f(x: Int) -> Int { return x(); }");
}

TEST(IRGenTest, Conditionals)
{
    // else case is optional
    valid("public func f(x: Int) -> Int { if (x % 2 == 0) return 1337; "
        "return x; }");
    // if/else can be nested
    valid("public func f(x: Int) -> Int "
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
    valid("public func f(x: Int) -> Int { if (x == 0) return 0; "
        "else if (x == 1) return 1; else return x; }");
    valid("public func fibonacci(x: Int) -> Int "
        "{ "
            "if (x <= 0) return 0; "
            "else if (x == 1) return 1; "
            "else return fibonacci(x: x - 1) + fibonacci(x: x - 2); "
        "}");
    // make sure ternary works
    valid("public func f(x: Int) -> Int { return x == 4 ? x : x + 1; }");
    // ternaries can also be chained on both sides of the colon without parens
    valid("public func f(x: Int) -> Int { return x == 4 ? "
        "x == 3 ? x : x + 1 :"
            "x == 2 ? x + 2 : x + 3; }");
}

TEST(IRGenTest, Variables)
{
    valid("public func f(x: Int) -> Int { let y: Int = x * 2; y = y / x; "
        "return y; }");
    valid("public var x: Int = f(); private func f() -> Int { return 2; }");
    valid("public var x: Int = 4; public func f() -> Int { return x + 1; }");
    valid("private var x: Int = 3; public var y: Int = x + 2;");
    valid("private var x: Int = 10; public func f(y: Int) -> Void { x = y; }");
    // can't access a global variable before it gets initialized
    invalid("public var x: Int = z; public var z: Int = 1;");
    // not implemented yet
    invalid("public func f() -> Int { return x + 1; } public var x: Int = 2;");
}

TEST(IRGenTest, Classes)
{
    // constructor/method bodies
    valid("public class X{ public var x: Int; "
        "public init(x: Int){ self.x = x; } "
        "public func f() -> Int { return self.x + 1; } }");
    // global variables of class type
    valid("public class X { public let x: Int; public init(){} } "
        "public let y: X = X();");
    // modifying the fields of an object
    valid("public class X { public let x: Int; public init(){} } "
        "public func f(x: X) -> Void { x.x = x.x + 1; }");
}

TEST(IRGenTest, MethodCalls)
{
    valid("public class X { "
        "public func f(x: X) -> X { return x.f(x: x); } }");
    // mismatched return type
    invalid("public class X { "
        "public func f(x: X) -> Int { return self; } }");
}

TEST(IRGenTest, Self)
{
    // method that returns self
    valid("public class X { public func f() -> X { return self; } }");
    // self is used outside of a class
    invalid("public func f() -> Int { return self.x; }");
}

TEST(IRGenTest, MemberAccess)
{
    // private field
    valid("public class A { private var x: Int; "
        "public func f() -> Int { return self.x; } }");
    invalid("public class A { private var x: Int; } "
        "public func f(x: A) -> Void { x.x = 1; }");
    // private ctor
    valid("public class A { private init(){} "
        "public func f() -> A { return A(); } }");
    invalid("public class A { private init(){} } "
        "public var a: A = A();");
    // private method
    valid("public class A { private func f() -> Void {} "
        "public func g() -> Void { self.f(); } }");
    invalid("public class A { private func f() -> Void {} } "
        "public func f(a: A) -> Void { a.f(); }");
}
