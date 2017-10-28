#ifndef LEXER_HPP
#define LEXER_HPP

#include "lexer/token.hpp"
#include <memory>

/**
 * Base class for lexers.
 */
class Lexer
{
public:
    /**
     * Destroys a Lexer.
     */
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
};

#endif // LEXER_HPP
