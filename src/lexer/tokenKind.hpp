#ifndef TOKENKIND_HPP
#define TOKENKIND_HPP

#include "llvm/ADT/StringRef.h"

/**
 * Specifies the kind of token that a {@link Token} can be. They can be found in
 * the file `lexer/tokenKind.def`.
 */
enum class TokenKind
{
    // tokenKind.def automatically #undef's the macros it uses
#define TOKEN(X, Y) X,
#include "lexer/tokenKind.def"
    /** The number of TokenKinds that exist. */
    COUNT
};

/**
 * Gets the external name of a TokenKind. It's best to use this rather than
 * {@link tokenKindName} wherever possible in diagnostic messages.
 *
 * @param k The TokenKind to get the external name of.
 *
 * @returns The external name of k.
 */
const char* tokenKindName(TokenKind k);

/**
 * Gets the internal name of a TokenKind. These shouldn't be used in diagnostic
 * messages but it's fine for debugging and what not.
 *
 * @param k The TokenKind to get the internal name of.
 *
 * @returns The internal name of k.
 */
const char* tokenKindDebugName(TokenKind k);

/**
 * Matches the given string with the appropriate keyword. If no keyword could be
 * matched, returns identifier.
 *
 * @param s The string to match with a keyword.
 *
 * @returns The corresponding keyword TokenKind, or identifier if not a keyword.
 */
TokenKind getKeywordKind(llvm::StringRef s);

#endif // TOKENKIND_HPP
