#ifndef PARSER_HPP
#define PARSER_HPP

#include "node.hpp"
#include "token.hpp"
#include <cstddef>
#include <memory>
#include <vector>

class Parser
{
public:
    Parser(std::vector<std::unique_ptr<Token>> tokens);
    std::unique_ptr<Node> parse();

private:
    const Token& next();
    const Token& current() const;
    const Token& peek(size_t i = 1) const;
    std::vector<std::unique_ptr<Node>> parseStatements();
    std::unique_ptr<Node> parseStatement();
    std::unique_ptr<EmptyNode> parseEmptyStatement();
    std::unique_ptr<BlockNode> parseBlock();
    std::unique_ptr<ConditionalNode> parseConditional();
    std::unique_ptr<AssignmentNode> parseAssignment();
    std::unique_ptr<TypeNode> parseType();
    std::unique_ptr<ExprNode> parseExpr(int rbp = 0);
    std::unique_ptr<ExprNode> parseNud();
    std::unique_ptr<ExprNode> parseLed(std::unique_ptr<ExprNode> left);
    int getLbp(const Token& token) const;
    std::unique_ptr<CallExprNode> parseCall(std::unique_ptr<ExprNode> callee);
    std::unique_ptr<ArgNode> parseCallArg();
    std::vector<std::unique_ptr<Token>> tokens;
    std::vector<std::unique_ptr<Token>>::iterator pos;
};

#endif // PARSER_HPP
