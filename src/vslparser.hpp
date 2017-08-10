#ifndef VSLPARSER_HPP
#define VSLPARSER_HPP

#include "lexer.hpp"
#include "node.hpp"
#include "parser.hpp"
#include "token.hpp"
#include <cstddef>
#include <deque>
#include <iostream>
#include <memory>

class VSLParser : public Parser
{
public:
    VSLParser(Lexer& lexer, std::ostream& errors = std::cerr);
    virtual ~VSLParser() override = default;
    virtual std::unique_ptr<Node> parse() override;

private:
    const Token& next();
    const Token& current();
    const Token& peek(size_t i = 1);
    std::unique_ptr<Node> errorExpected(const char* s);
    std::unique_ptr<Node> errorUnexpected(const Token& token);
    std::vector<std::unique_ptr<Node>> parseStatements();
    std::unique_ptr<Node> parseStatement();
    std::unique_ptr<Node> parseEmptyStatement();
    std::unique_ptr<Node> parseBlock();
    std::unique_ptr<Node> parseConditional();
    std::unique_ptr<Node> parseAssignment();
    std::unique_ptr<Node> parseFunction();
    std::unique_ptr<Node> parseReturn();
    FunctionNode::Param parseParam();
    std::unique_ptr<Type> parseType();
    std::unique_ptr<Node> parseExpr(int rbp = 0);
    std::unique_ptr<Node> parseNud();
    std::unique_ptr<Node> parseLed(std::unique_ptr<Node> left);
    int getLbp(const Token& token) const;
    std::unique_ptr<Node> parseCall(std::unique_ptr<Node> callee);
    std::unique_ptr<Node> parseCallArg();
    Lexer& lexer;
    std::deque<std::unique_ptr<Token>> cache;
    std::ostream& errors;
};

#endif // VSLPARSER_HPP
