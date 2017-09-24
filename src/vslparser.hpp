#ifndef VSLPARSER_HPP
#define VSLPARSER_HPP

#include "lexer.hpp"
#include "node.hpp"
#include "parser.hpp"
#include "token.hpp"
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
     * @param lexer The Lexer to get the tokens from.
     * @param errors The stream to print errors to.
     */
    VSLParser(Lexer& lexer, std::ostream& errors = std::cerr);
    /**
     * Destroys a VSLParser.
     */
    virtual ~VSLParser() override = default;
    virtual std::unique_ptr<Node> parse() override;
    /**
     * Checks if the lexer has encountered an error yet. In this case, a warning
     * would also count as an error.
     *
     * @returns True if the lexer encountered an error, false otherwise.
     */
    bool hasError() const;

private:
    /**
     * Gets the next token, consuming the current one.
     *
     * @returns The next token.
     */
    const Token& next();
    /**
     * Gets the current token.
     *
     * @returns The current token.
     */
    const Token& current();
    /**
     * Prints an error saying that the parser expected `s` but was given
     * something else.
     *
     * @param s What the parser was originally expecting.
     *
     * @returns An {@link ErrorNode}.
     */
    std::unique_ptr<Node> errorExpected(const char* s);
    /**
     * Prints an error saying that the parser didn't expect the given token.
     *
     * @param token The token that the parser didn't expect.
     *
     * @returns An {@link ErrorNode}.
     */
    std::unique_ptr<Node> errorUnexpected(const Token& token);
    /**
     * Parses a sequence of statements.
     *
     * @returns A sequence of statements.
     */
    std::vector<std::unique_ptr<Node>> parseStatements();
    /**
     * Parses a single statement.
     *
     * @returns A single statement.
     */
    std::unique_ptr<Node> parseStatement();
    /**
     * Parses an empty statement.
     *
     * @returns An empty statement.
     */
    std::unique_ptr<Node> parseEmptyStatement();
    /**
     * Parses a block of code.
     *
     * @returns A block of code.
     */
    std::unique_ptr<Node> parseBlock();
    /**
     * Parses an if/else statement.
     *
     * @returns An if/else statement.
     */
    std::unique_ptr<Node> parseConditional();
    /**
     * Parses a variable declaration.
     *
     * @returns A variable declaration.
     */
    std::unique_ptr<Node> parseAssignment();
    /**
     * Parses a function.
     *
     * @returns A function.
     */
    std::unique_ptr<Node> parseFunction();
    /**
     * Parses a return statement.
     *
     * @returns A return statement.
     */
    std::unique_ptr<Node> parseReturn();
    /**
     * Parses a function parameter.
     *
     * @returns A function parameter.
     */
    FunctionNode::Param parseParam();
    /**
     * Parses a VSL type.
     *
     * @returns A VSL type.
     */
    std::unique_ptr<Type> parseType();
    /**
     * Parses an expression.
     *
     * @param rbp The right binding power to use when parsing. Only use this if
     * you understand Pratt parsing.
     *
     * @returns An expression.
     */
    std::unique_ptr<Node> parseExpr(int rbp = 0);
    /**
     * Parses the null denotation of the current token, or when there is no
     * expression to the left of the current token that has a higher right
     * binding power than it. Only use this if you understand Pratt parsing.
     *
     * @returns The null denotation of the current token.
     */
    std::unique_ptr<Node> parseNud();
    /**
     * Parses the left denotation of the current token, or when there is an
     * expression to the left of the current token that has a higher right
     * binding power than it. Only use this if you understand Pratt parsing.
     *
     * @param left The left hand side of the expression.
     *
     * @returns The left denotation of the current token.
     */
    std::unique_ptr<Node> parseLed(std::unique_ptr<Node> left);
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
     * Parses a function call.
     *
     * @param callee The function to call.
     *
     * @returns A function call.
     */
    std::unique_ptr<Node> parseCall(std::unique_ptr<Node> callee);
    /**
     * Parses a function argument.
     *
     * @returns A function argument.
     */
    std::unique_ptr<Node> parseCallArg();
    /** The Lexer to get the tokens from. */
    Lexer& lexer;
    /** Cache of tokens used in lookahead. */
    std::deque<std::unique_ptr<Token>> cache;
    /** The stream to print errors to. */
    std::ostream& errors;
    /** True if the lexer encountered an error, otherwise false. */
    bool errored;
};

#endif // VSLPARSER_HPP
