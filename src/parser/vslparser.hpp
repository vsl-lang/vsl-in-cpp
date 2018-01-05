#ifndef VSLPARSER_HPP
#define VSLPARSER_HPP

#include "ast/node.hpp"
#include "ast/vslContext.hpp"
#include "diag/diag.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/parser.hpp"
#include <cstddef>
#include <deque>
#include <memory>
#include <vector>

/**
 * Parser for VSL.
 */
class VSLParser : public Parser
{
public:
    /**
     * Creates a VSLParser.
     *
     * @param vslContext The VSLContext object to be used.
     * @param lexer The Lexer to get the tokens from.
     */
    VSLParser(VSLContext& vslContext, Lexer& lexer);
    /**
     * Destroys a VSLParser.
     */
    virtual ~VSLParser() override = default;
    virtual std::vector<Node*> parse() override;

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
     *
     * @returns An empty node.
     */
    EmptyNode* errorExpected(const char* s);
    /**
     * Prints an error saying that the parser didn't expect the given token.
     *
     * @param token The token that the parser didn't expect.
     *
     * @returns An empty node.
     */
    EmptyNode* errorUnexpected(const Token& token);

    /**
     * @}
     * @name Global Scope Parsing
     * @{
     */

    /**
     * Parses a sequence of global declarations, e.g.\ functions and what not.
     *
     * @returns A sequence of global declarations.
     */
    std::vector<Node*> parseGlobals();
    /**
     * Parses a function, e.g.\ `func f(x: Int) -> Int { ... }`.
     *
     * @returns A function.
     */
    Node* parseFunction();
    /**
     * Parses a function parameter, e.g.\ `x: Int`.
     *
     * @returns A function parameter.
     */
    ParamNode* parseParam();

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
     * Parses a statement within a function scope. Functions are not allowed
     * here.
     *
     * @returns A single statement.
     */
    Node* parseStatement();
    /**
     * Parses a block of code, e.g.\ `{ statements... }`.
     *
     * @returns A block of code.
     */
    Node* parseBlock();
    /**
     * Parses an if/else statement, e.g.\ `if (x) { y; } else { z; }`.
     *
     * @returns An if/else statement.
     */
    Node* parseIf();
    /**
     * Parses a variable declaration, e.g.\ `var x: Int = 1;`.
     *
     * @returns A variable declaration.
     */
    Node* parseVariable();
    /**
     * Parses a return statement, e.g.\ `return 1;` or just `return;`.
     *
     * @returns A return statement.
     */
    Node* parseReturn();
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
    ExprNode* parseBinaryOp(ExprNode* lhs);
    /**
     * Parses a binary expression.
     *
     * @param lhs Already parsed left hand side.
     *
     * @returns A binary expression.
     */
    ExprNode* parseBinaryExpr(ExprNode* lhs);
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
     * @param condition Condition that was already parsed.
     *
     * @returns A ternary expression.
     */
    TernaryNode* parseTernary(ExprNode* condition);
    /**
     * Parses a function call, e.g.\ `f(x: 1)`.
     *
     * @param callee The function to call.
     *
     * @returns A function call.
     */
    CallNode* parseCall(ExprNode* callee);
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

    /** @} */

    /** Reference to the VSLContext. */
    VSLContext& vslContext;
    /** The Lexer to get the tokens from. */
    Lexer& lexer;
    /** Diagnostics manager. */
    Diag& diag;
    /** Cache of tokens used in lookahead. */
    std::deque<Token> cache;
};

#endif // VSLPARSER_HPP
