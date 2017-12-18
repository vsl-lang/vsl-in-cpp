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
 * Represents a lexer token.
 */
class Token
{
    friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os,
        const Token& token);
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
    /**
     * Gets the TokenKind that this Token represents.
     *
     * @returns The kind of Token this is.
     */
    TokenKind getKind() const;
    /**
     * Verifies whether this Node represents a certain Kind.
     *
     * @param k The Kind to check for.
     *
     * @returns True if this Node represents the given Kind, false otherwise.
     */
    bool is(TokenKind k) const;
    bool isNot(TokenKind k) const;
    /**
     * Gets the TokenKind in a more human-readable representation for diagnostic
     * messages and stuff.
     *
     * @returns The TokenKind in string form.
     */
    const char* getKindName() const;
    /**
     * Gets the text that this Token represents.
     *
     * @returns The text that this Token represents.
     */
    llvm::StringRef getText() const;
    /**
     * Gets the location info.
     *
     * @returns Where this Node was found in the source.
     */
    Location getLoc() const;

private:
    /** The kind of Token this is. */
    TokenKind kind;
    /** The text that was found in the Token. */
    llvm::StringRef text;
    /** Where this Token was found in the source. */
    Location location;
};

#endif // TOKEN_HPP
