#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "lexer/location.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <unordered_map>

class Token;

/**
 * Allows a {@link Token} to be printed to an output stream.
 *
 * @param os The stream to print to.
 * @param token The Token to print.
 *
 * @returns The same output stream that was given.
 */
std::ostream& operator<<(std::ostream& os, const Token& token);

/**
 * Represents a lexer token.
 */
class Token
{
public:
    /**
     * Identifies the kind of Token a given object is. Keep in mind that one
     * subclass can represent multiple Kinds of Tokens.
     */
    enum Kind
    {
        /** Number. */
        NUMBER,
        /** Identifier. */
        IDENTIFIER,
        /** `var`. */
        KEYWORD_VAR,
        /** `let`. */
        KEYWORD_LET,
        /** `func`. */
        KEYWORD_FUNC,
        /** `return`. */
        KEYWORD_RETURN,
        /** `if`. */
        KEYWORD_IF,
        /** `else`. */
        KEYWORD_ELSE,
        /** `Int`. */
        KEYWORD_INT,
        /** `Void`. */
        KEYWORD_VOID,
        /** `+`. */
        OP_PLUS,
        /** `-`. */
        OP_MINUS,
        /** `*`. */
        OP_STAR,
        /** `/`. */
        OP_SLASH,
        /** `%`, or modulo. */
        OP_PERCENT,
        /** `=`. */
        OP_ASSIGN,
        /** `==`. */
        OP_EQUALS,
        /** `>`. */
        OP_GREATER,
        /** `>=`. */
        OP_GREATER_EQUAL,
        /** `<`. */
        OP_LESS,
        /** `<=`. */
        OP_LESS_EQUAL,
        /** `:`. */
        SYMBOL_COLON,
        /** `;`. */
        SYMBOL_SEMICOLON,
        /** `,`. */
        SYMBOL_COMMA,
        /** `->`. */
        SYMBOL_ARROW,
        /** `(`. */
        SYMBOL_LPAREN,
        /** `)`. */
        SYMBOL_RPAREN,
        /** `{`. */
        SYMBOL_LBRACE,
        /** `}`. */
        SYMBOL_RBRACE,
        /** End of file. */
        SYMBOL_EOF
    };
    /**
     * Creates a Token.
     *
     * @param kind The kind of Token this is.
     * @param location Where this Token was found in the source.
     */
    Token(Kind kind, Location location);
    /**
     * Destroys a Token.
     */
    virtual ~Token() = 0;
    /**
     * Gets the string representation of a given Kind.
     *
     * @param kind The Kind to get the string representation for.
     *
     * @returns The string representation of a given Kind.
     */
    static const char* kindToString(Kind kind);
    /**
     * Returns the string representation of the Token.
     *
     * @returns The string representation of the Token.
     */
    virtual std::string toString() const = 0;
    /** The kind of Token this is. */
    Kind kind;
    /** Where this Token was found in the source. */
    Location location;
};

/**
 * Represents a simple Token with no extra data.
 */
class DefaultToken : public Token
{
public:
    /**
     * Creates a DefaultToken.
     *
     * @param kind The kind of DefaultToken this is.
     * @param location Where this DefaultToken was found in the source.
     */
    DefaultToken(Kind kind, Location location);
    /**
     * Destroys a DefaultToken.
     */
    virtual ~DefaultToken() override = default;
    virtual std::string toString() const override;
};

/**
 * Represents an identifier or keyword.
 */
class NameToken : public Token
{
public:
    /**
     * Creates a NameToken.
     *
     * @param name The name of the identifier or keyword.
     * @param location Where this NameToken was found in the source.
     */
    NameToken(std::string name, Location location);
    /**
     * Destroys a NameToken.
     */
    virtual ~NameToken() override = default;
    virtual std::string toString() const override;
    /** The name of the identifier or keyword. */
    std::string name;

private:
    /**
     * Determines whether the given string is either a special keyword or just a
     * regular identifier.
     *
     * @param name The name of the identifier or keyword.
     *
     * @returns One of the `KEYWORD_*` constants for a keyword or `IDENTIFIER`
     * for a regular identifier.
     */
    static Token::Kind evaluateName(const std::string& name);
    /** Keeps track of all the VSL keywords. */
    static const std::unordered_map<std::string, Token::Kind> keywords;
};

/**
 * Represents a number. For now it's just a long int.
 */
class NumberToken : public Token
{
public:
    /**
     * Creates a NumberToken.
     *
     * @param value The value of the number.
     * @param location Where this NumberToken was found in the source.
     */
    NumberToken(long value, Location location);
    /**
     * Destroys a NumberToken.
     */
    virtual ~NumberToken() override = default;
    virtual std::string toString() const override;
    /** The value of the number. */
    long value;
};

#endif // TOKEN_HPP
