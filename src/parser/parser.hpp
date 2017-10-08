#ifndef PARSER_HPP
#define PARSER_HPP

#include "ast/node.hpp"
#include <memory>

/**
 * Base class for parsers.
 */
class Parser
{
public:
    /**
     * Destroys a Parser.
     */
    virtual ~Parser() = 0;
    /**
     * Parses the program.
     *
     * @returns The AST of the program.
     */
    virtual std::unique_ptr<Node> parse() = 0;
};

#endif // PARSER_HPP
