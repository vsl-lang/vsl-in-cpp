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
     * @returns The AST of the program, wrapped in a {@link BlockNode}.
     */
    virtual std::unique_ptr<BlockNode> parse() = 0;
};

#endif // PARSER_HPP
