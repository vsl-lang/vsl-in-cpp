#ifndef VSLPARSER_HPP
#define VSLPARSER_HPP

#include "ast/node.hpp"
#include "diag/diag.hpp"
#include "irgen/vslContext.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/parser.hpp"
#include <cstddef>
#include <deque>
#include <iostream>
#include <memory>

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
    virtual BlockNode* parse() override;

private:
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
     * Parses a sequence of statements. Each statement production must consume
     * all tokens involved in that production, and each production must start
     * with `current()` being the first token consumed.
     *
     * @returns A sequence of statements.
     */
    std::vector<Node*> parseStatements();
    /**
     * Parses a single statement.
     *
     * @returns A single statement.
     */
    Node* parseStatement();
    /**
     * Parses an empty statement, e.g.\ `;`.
     *
     * @returns An empty statement.
     */
    EmptyNode* parseEmptyStatement();
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
     * Parses a return statement, e.g.\ `return 1;` or just `return;`.
     *
     * @returns A return statement.
     */
    Node* parseReturn();
    /**
     * Parses an expression, e.g.\ `1+5*(3+4)-6/2`.
     *
     * @param rbp The right binding power to use when parsing. Only use this if
     * you understand Pratt parsing.
     *
     * @returns An expression.
     */
    ExprNode* parseExpr(int rbp = 0);
    /**
     * Parses the null denotation of the current token, or when there is no
     * expression to the left of the current token that has a higher right
     * binding power than it. Only use this if you understand Pratt parsing.
     *
     * @returns The null denotation of the current token.
     */
    ExprNode* parseNud();
    /**
     * Parses the left denotation of the current token, or when there is an
     * expression to the left of the current token that has a higher right
     * binding power than it. Only use this if you understand Pratt parsing.
     *
     * @param left The left hand side of the expression.
     *
     * @returns The left denotation of the current token.
     */
    ExprNode* parseLed(ExprNode* left);
    /**
     * Gets the left binding power of a given token. Only use this if you
     * understand Pratt parsing.
     *
     * @param token The token to get the right binding power for.
     *
     * @returns The left binding power of a given token.
     */
    int getLbp(const Token& token) const;
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
     * Parses a VSL type, e.g.\ `Int` or `Void`.
     *
     * @returns A VSL type.
     */
    const Type* parseType();
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
