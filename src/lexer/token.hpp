#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "lexer/location.hpp"
#include "lexer/tokenKind.hpp"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <cstddef>
#include <string>
#include <unordered_map>

class Token;

/**
 * Allows a Token to be printed to an output stream.
 *
 * @param os The stream to print to.
 * @param token The Token to print.
 *
 * @returns The same output stream that was given.
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const Token& token);

/**
 * Represents a lexer token.
 */
class Token
{
public:
    /**
     * Creates a Token.
     */
    Token();
    /**
     * Creates a Token.
     *
     * @param kind The kind of Token this is.
     * @param text Tbe text that was found in the Token.
     * @param location Where this Token was found in the source.
     */
    Token(TokenKind kind, llvm::StringRef text, Location location);
    /** The kind of Token this is. */
    TokenKind kind;
    /** The text that was found in the Token. */
    llvm::StringRef text;
    /** Where this Token was found in the source. */
    Location location;
};

#endif // TOKEN_HPP
