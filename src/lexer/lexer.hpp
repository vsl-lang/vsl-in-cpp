#ifndef LEXER_HPP
#define LEXER_HPP

#include "diag/diag.hpp"
#include "lexer/token.hpp"
#include <memory>

/**
 * Base class for lexers.
 */
class Lexer
{
public:
    /**
     * Creates a Lexer.
     *
     * @param diag Diagnostics manager.
     */
    Lexer(Diag& diag);
    virtual ~Lexer() = 0;
    /**
     * Gets the next token.
     *
     * @returns The next token.
     */
    virtual Token nextToken() = 0;
    /**
     * Tests if this Lexer is all out of tokens.
     *
     * @returns True if empty, otherwise false.
     */
    virtual bool empty() const = 0;
    /**
     * Gets the diagnostics manager.
     *
     * @returns The diagnostics manager.
     */
    Diag& getDiag() const;

protected:
    /** Diagnostics manager. */
    Diag& diag;
};

#endif // LEXER_HPP
