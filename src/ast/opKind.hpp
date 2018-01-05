#ifndef OPKIND_HPP
#define OPKIND_HPP

#include "lexer/tokenKind.hpp"

/**
 * Unary operators. See `ast/opKind.def`.
 */
enum class UnaryKind
{
    // the included file automatically #undef's the macros it uses
    /** @cond */
#define UNARY(kind, name) kind,
    /** @endcond */
#include "ast/opKind.def"
    /** Amount of UnaryKinds that exist. */
    COUNT
};

/**
 * Binary operators. See `ast/opKind.def`.
 */
enum class BinaryKind
{
    /** @cond */
#define BINARY(kind, name) kind,
    /** @endcond */
#include "ast/opKind.def"
    /** Amount of BinaryKinds that exist. */
    COUNT
};

/**
 * @name String Conversions
 * @{
 */

/**
 * Gets the symbol of the unary operator.
 *
 * @param k Kind of unary operation.
 *
 * @returns Symbol of the given UnaryKind.
 */
const char* unaryKindSymbol(UnaryKind k);

/**
 * Gets the symbol of the binary operator.
 *
 * @param k Kind of binary operation.
 *
 * @returns Symbol of the given BinaryKind.
 */
const char* binaryKindSymbol(BinaryKind k);

/**
 * @}
 * @name TokenKind Conversions
 * @{
 */

/**
 * Converts a TokenKind to a UnaryKind.
 *
 * @param k TokenKind to convert.
 *
 * @returns An appropriate UnaryKind if possible, invalid otherwise.
 */
UnaryKind tokenKindToUnary(TokenKind k);

/**
 * Converts a TokenKind to a BinaryKind.
 *
 * @param k TokenKind to convert.
 *
 * @returns An appropriate BinaryKInd if possible, invalid otherwise.
 */
BinaryKind tokenKindToBinary(TokenKind k);

/** @} */

#endif // OPKIND_HPP
