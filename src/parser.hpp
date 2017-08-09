#ifndef PARSER_HPP
#define PARSER_HPP

#include "node.hpp"
#include <memory>

class Parser
{
public:
    virtual ~Parser() = 0;
    virtual std::unique_ptr<Node> parse() = 0;
};

#endif // PARSER_HPP
