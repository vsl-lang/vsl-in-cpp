#ifndef VSLLEXER_HPP
#define VSLLEXER_HPP

#include "lexer/lexer.hpp"
#include "lexer/location.hpp"
#include "lexer/token.hpp"
#include <cstddef>
#include <iostream>
#include <memory>

/**
 * Lexer for VSL.
 */
class VSLLexer : public Lexer
{
public:
    /**
     * Creates a VSLLexer.
     *
     * @param src The source code to lex.
     * @param errors The stream to print errors to.
     */
    VSLLexer(const char* src, std::ostream& errors = std::cerr);
    /**
     * Destroys a VSLLexer.
     */
    virtual ~VSLLexer() override = default;
    virtual std::unique_ptr<Token> nextToken() override;
    virtual bool empty() const override;
    /**
     * Checks if an error has been encountered yet.
     *
     * @returns True if an error was encountered, false otherwise.
     */
    bool hasError() const;

private:
    /**
     * Gets the current character.
     *
     * @returns The current character.
     */
    char current() const;
    /**
     * Gets the next character, consuming the current one.
     *
     * @returns The next character.
     */
    char next();
    /**
     * Gets the `i`th character after the current one, without consuming any.
     *
     * @param i The index of the character.
     *
     * @returns The `i`th character after the current one.
     */
    char peek(size_t i = 1) const;
    /**
     * Creates a {@link DefaultToken}.
     *
     * @param kind The kind of DefaultToken to create.
     *
     * @returns A new {@link DefaultToken}.
     */
    std::unique_ptr<DefaultToken> lexDefaultToken(Token::Kind kind);
    /**
     * Lexes a {@link NameToken}.
     *
     * @returns A new {@link NameToken}.
     */
    std::unique_ptr<NameToken> lexName();
    /**
     * Lexes a {@link NumberToken}.
     *
     * @returns A new {@link NumberToken}.
     */
    std::unique_ptr<NumberToken> lexNumber();
    /**
     * Consumes a line comment.
     */
    void lexLineComment();
    /**
     * Consumes a block comment.
     */
    void lexBlockComment();
    /**
     * The complete source code of the program.
     */
    const char* src;
    /**
     * The location of the current character.
     */
    Location location;
    /**
     * The stream to print errors to.
     */
    std::ostream& errors;
    /**
     * True if an error was encountered, otherwise false.
     */
    bool errored;
};

#endif // VSLLEXER_HPP
