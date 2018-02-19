#ifndef VSLPARSER_HPP
#define VSLPARSER_HPP

#include "ast/node.hpp"
#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "llvm/ADT/ArrayRef.h"
#include <cstddef>
#include <memory>
#include <type_traits>

/**
 * Parser for VSL.
 */
class VSLParser
{
public:
    /**
     * Creates a VSLParser.
     *
     * @param vslCtx The VSLContext object to be used.
     * @param lexer The Lexer to get the tokens from.
     */
    VSLParser(VSLContext& vslCtx, Lexer& lexer);
    /**
     * Parses the program. The AST is stored in the VSLContext.
     */
    void parse();

private:
    /**
     * @name Token Operations
     * @{
     */

    /**
     * Gets the current token while consuming it. Any previous references
     * returned by `current()` will be invalidated.
     *
     * @returns The current token by value.
     */
    Token consume();
    /**
     * Gets the current token without consuming it. This is equivalent to
     * calling `peek(0)`.
     *
     * @returns The current token.
     */
    const Token& current();
    /**
     * Look `depth` tokens ahead of the current token without consuming them.
     *
     * @param depth The amount of tokens to look ahead.
     *
     * @returns The next token without consuming it.
     */
    const Token& peek(size_t depth = 1);
    /**
     * Checks if both the lexer and the token cache are empty. When this is
     * true, all new tokens created by the lexer will typically be eof tokens.
     *
     * @returns True if empty, false otherwise.
     */
    bool empty() const;

    /**
     * @}
     * @name Diagnostics Helpers
     * @{
     */

    /**
     * Prints an error saying that the parser expected `s` but was given
     * something else.
     *
     * @param s What the parser was originally expecting.
     */
    void errorExpected(const char* s);
    /**
     * Prints an error saying that the parser didn't expect the given token.
     *
     * @param token The token that the parser didn't expect.
     */
    void errorUnexpected(const Token& token);

    /**
     * @}
     * @name Global Scope Parsing
     * @{
     */

    /**
     * Parses a declaration with an access specifier. This can be a free
     * function or a global variable.
     *
     * @returns A declaration.
     */
    DeclNode* parseDecl();
    /**
     * Parses a function, e.g.\ `public func f(x: Int) -> Int { ... }`. External
     * functions are also included here.
     *
     * @param access Access modifier.
     *
     * @returns A function.
     */
    FuncInterfaceNode* parseFunction(AccessMod access);
    /**
     * Parses a function parameter, e.g.\ `x: Int`.
     *
     * @returns A function parameter.
     */
    ParamNode* parseParam();
    /**
     * Parses a variable declaration, e.g.\ `public var x: Int = 1;`. This can
     * also be inside a function where there is no access modifier.
     *
     * @param access Access modifier.
     *
     * @returns A variable declaration.
     */
    VariableNode* parseVariable(AccessMod access = AccessMod::NONE);

    /**
     * @}
     * @name Statement Parsing
     * @{
     */

    /**
     * Parses a sequence of statements. Each statement production must consume
     * all tokens involved in that production, and each production must start
     * with `current()` being the first token consumed.
     *
     * @returns A sequence of statements.
     */
    std::vector<Node*> parseStatements();
    /**
     * Parses a statement within a function scope. It is assumed that this is
     * within a function scope, therefore functions are not allowed here.
     *
     * @returns A single statement.
     */
    Node* parseStatement();
    /**
     * Parses a block of code, e.g.\ `{ statements... }`.
     *
     * @returns A block of code.
     */
    BlockNode* parseBlock();
    /**
     * Parses an if/else statement, e.g.\ `if (x) { y; } else { z; }`.
     *
     * @returns An if/else statement.
     */
    IfNode* parseIf();
    /**
     * Parses a return statement, e.g.\ `return 1;` or just `return;`.
     *
     * @returns A return statement.
     */
    ReturnNode* parseReturn();
    /**
     * Parses an expression statement ending in a semicolon, e.g.\ `x = 1;`.
     *
     * @returns An expression statement.
     */
    ExprNode* parseExprStmt();

    /**
     * @}
     * @name Expression Parsing
     * @{
     */

    /**
     * Parses an expression, e.g.\ `1+5*(3+4)-6/2`.
     *
     * @param minPrec Minimum precedence allowed when parsing binary operators.
     *
     * @returns An expression.
     */
    ExprNode* parseExpr(int minPrec = 0);
    /**
     * Parses the current token when there is no expression of higher precedence
     * to the left of it.
     *
     * @returns A unary expression, literal, identifier, or paren group.
     */
    ExprNode* parseUnaryOp();
    /**
     * Parses the current token when there is an expression of higher precedence
     * to the left of it. This expression is passed as a parameter.
     *
     * @param lhs Left hand side of the expression.
     *
     * @returns A binary, ternary, or call expression.
     */
    ExprNode* parseBinaryOp(ExprNode& lhs);
    /**
     * Parses a binary expression.
     *
     * @param lhs Already parsed left hand side.
     *
     * @returns A binary expression.
     */
    BinaryNode* parseBinaryExpr(ExprNode& lhs);
    /**
     * Gets the precedence of a binary (or ternary/kind) operator.
     *
     * @param k The kind of operator to use.
     *
     * @returns The precedence of the given operator.
     */
    static int getPrec(TokenKind k);
    /**
     * Parses a ternary expression, e.g.\ `c ? x : y`.
     *
     * @param condition Condition that was already parsed. Cannot be null.
     *
     * @returns A ternary expression.
     */
    TernaryNode* parseTernary(ExprNode& condition);
    /**
     * Parses a function call, e.g.\ `f(x: 1)`.
     *
     * @param callee The function to call. Cannot be null.
     *
     * @returns A function call.
     */
    CallNode* parseCall(ExprNode& callee);
    /**
     * Parses a function argument, e.g.\ `x: 1`.
     *
     * @returns A function argument.
     */
    ArgNode* parseCallArg();
    /**
     * Parses a number, e.g.\ `1337`.
     *
     * @param token The token to get the number from.
     *
     * @returns A number expression.
     */
    LiteralNode* parseNumber(const Token& token);

    /**
     * @}
     * @name Other Helpers
     * @{
     */

    /**
     * Parses a VSL type, e.g.\ `Int` or `Void`.
     *
     * @returns A VSL type.
     */
    const Type* parseType();

    /**
     * Creates a Node.
     *
     * @tparam NodeT The Node-derived type to instantiate.
     * @tparam Args NodeT's constructor arguments.
     *
     * @param args NodeT's constructor arguments.
     *
     * @returns A pointer to a newly created NodeT.
     */
    template<typename NodeT, typename... Args>
    typename std::enable_if<std::is_base_of<Node, NodeT>::value, NodeT*>::type
    makeNode(Args&&... args) const;

    /** @} */

    /** Reference to the VSLContext. */
    VSLContext& vslCtx;
    /** The Lexer to get the tokens from. */
    Lexer& lexer;
    /** Diagnostics manager. */
    Diag& diag;
    /** Cache of tokens used in lookahead. */
    std::deque<Token> cache;
};

#endif // VSLPARSER_HPP
