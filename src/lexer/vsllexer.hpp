#ifndef VSLLEXER_HPP
#define VSLLEXER_HPP

#include "diag/diag.hpp"
#include "irgen/vslContext.hpp"
#include "lexer/lexer.hpp"
#include "lexer/location.hpp"
#include "lexer/token.hpp"
#include "llvm/ADT/StringRef.h"

/**
 * Lexer for VSL.
 */
class VSLLexer : public Lexer
{
public:
    /**
     * Creates a VSLLexer.
     *
     * @param diag Diagnostics manager.
     * @param src The source code to lex.
     */
    VSLLexer(Diag& diag, const char* src);
    /**
     * Destroys a VSLLexer.
     */
    virtual ~VSLLexer() override = default;
    virtual Token nextToken() override;
    virtual bool empty() const override;

private:
    /**
     * Gets the newest (last) character from the text buffer.
     *
     * @returns The current character.
     */
    char current() const;
    /**
     * Gets the next character, adding it to the text buffer.
     *
     * @returns The next character.
     */
    char next();
    /**
     * Resets the text buffer so that it points to the character after where it
     * was pointing to previously. This should be done before and after creating
     * a Token.
     */
    void resetBuffer();
    /**
     * Gets the character after the current one, without consuming any.
     *
     * @returns The character after the current one.
     */
    char peek() const;
    /**
     * Creates a token with the specified TokenKind. Before returning,
     * this method will also setup the lexer for getting the next token.
     */
    Token createToken(TokenKind kind);
    /**
     * Creates a symbol token.
     *
     * @returns A symbol token.
     */
    Token lexSymbol();
    /**
     * Creates either an identifier or keyword token.
     *
     * @returns Either an identifier or keyword token.
     */
    Token lexIdentOrKeyword();
    /**
     * Creates a number literal token.
     *
     * @returns A number literal token.
     */
    Token lexNumber();
    /**
     * Consumes a line comment.
     */
    void lexLineComment();
    /**
     * Consumes a block comment.
     */
    void lexBlockComment();
    /** The current buffer of text to insert into the next Token. */
    llvm::StringRef text;
    /** The location of the current character. */
    Location location;
};

#endif // VSLLEXER_HPP
