#ifndef TOKENKIND_HPP
#define TOKENKIND_HPP

#include "llvm/ADT/StringRef.h"

/**
 * Specifies the kind of token that a {@link Token} can be. They can be found in
 * the file `lexer/tokenKind.def`.
 */
enum class TokenKind
{
#define TOKEN(X) X,
#include "lexer/tokenKind.def"
#undef TOKEN
    COUNT /** The number of TokenKinds that exist. */
};

/**
 * Gets the internal name of a TokenKind. These shouldn't be used in diagnostic
 * messages but it's fine for debugging and what not.
 *
 * @param tk The TokenKind to get the internal name of.
 *
 * @returns The internal name of tk.
 */
const char* getTokenKindName(TokenKind tk);

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
