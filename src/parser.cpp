#include "parser.hpp"
#include "node.hpp"
#include <cstddef>
#include <memory>
#include <vector>

Parser::Parser(std::vector<std::unique_ptr<Token>> tokens)
    : tokens{ std::move(tokens) }, pos{ this->tokens.begin() }
{
}

std::unique_ptr<Node> Parser::parse()
{
    size_t savedPos = current().getPos();
    auto root = std::make_unique<BlockNode>(parseStatements(), savedPos);
    if (current().getType() != Token::END)
    {
        return nullptr; // error
    }
    return root;
}

const Token& Parser::next()
{
    return **++pos;
}

const Token& Parser::current() const
{
    return **pos;
}

const Token& Parser::peek(size_t i) const
{
    return *pos[i];
}

std::vector<std::unique_ptr<Node>> Parser::parseStatements()
{
    std::vector<std::unique_ptr<Node>> statements;
    while (true)
    {
        switch (current().getType())
        {
        case Token::RBRACE:
        case Token::END:
            return statements;
        default:
            statements.emplace_back(parseStatement());
        }
    }
}

std::unique_ptr<Node> Parser::parseStatement()
{
    switch (current().getType())
    {
    case Token::IDENTIFIER:
    case Token::NUMBER:
    case Token::PLUS:
    case Token::MINUS:
    case Token::LPAREN:
        {
            std::unique_ptr<ExprNode> expr = parseExpr();
            if (current().getType() != Token::SEMICOLON)
            {
                return nullptr; // error
            }
            next();
            return expr;
        }
    case Token::LBRACE:
        return parseBlock();
    case Token::SEMICOLON:
        return parseEmptyStatement();
    default:
        return nullptr; // error
    }
}

std::unique_ptr<EmptyNode> Parser::parseEmptyStatement()
{
    const Token& semicolon = current();
    if (semicolon.getType() != Token::SEMICOLON)
    {
        return nullptr; // error
    }
    next();
    return std::make_unique<EmptyNode>(semicolon.getPos());
}

std::unique_ptr<ExprNode> Parser::parseExpr(int rbp)
{
    // top down operator precedence algorithm
    std::unique_ptr<ExprNode> left = parseNud();
    while (rbp < getLbp(current()))
    {
        left = parseLed(std::move(left));
    }
    return left;
}

std::unique_ptr<ExprNode> Parser::parseNud()
{
    const Token& token = current();
    next();
    switch (token.getType())
    {
    case Token::IDENTIFIER:
        return std::make_unique<IdentExprNode>(static_cast<const NameToken&>(
                token).getName(), token.getPos());
    case Token::NUMBER:
        return std::make_unique<NumberExprNode>(static_cast<const NumberToken&>(
                token).getValue(), token.getPos());
    case Token::PLUS:
    case Token::MINUS:
        return std::make_unique<UnaryExprNode>(token.getType(), parseExpr(100),
            token.getPos());
    default:
        return nullptr; // error
    }
}

std::unique_ptr<ExprNode> Parser::parseLed(std::unique_ptr<ExprNode> left)
{
    const Token& token = current();
    Token::Type type = token.getType();
    switch (type)
    {
    case Token::STAR:
    case Token::SLASH:
    case Token::PERCENT:
    case Token::PLUS:
    case Token::MINUS:
    case Token::GREATER:
    case Token::GREATER_EQUAL:
    case Token::LESS:
    case Token::LESS_EQUAL:
    case Token::EQUALS:
        next();
        return std::make_unique<BinaryExprNode>(type, std::move(left),
            parseExpr(getLbp(token)), token.getPos());
    case Token::ASSIGN:
        next();
        return std::make_unique<BinaryExprNode>(type, std::move(left),
            parseExpr(getLbp(token) - 1), token.getPos());
    case Token::LPAREN:
        return parseCall(std::move(left));
    default:
        return nullptr; // error
    }
}

int Parser::getLbp(const Token& token) const
{
    switch (token.getType())
    {
    case Token::LPAREN:
        return 6;
    case Token::STAR:
    case Token::SLASH:
    case Token::PERCENT:
        return 5;
    case Token::PLUS:
    case Token::MINUS:
        return 4;
    case Token::GREATER:
    case Token::GREATER_EQUAL:
    case Token::LESS:
    case Token::LESS_EQUAL:
        return 3;
    case Token::EQUALS:
        return 2;
    case Token::ASSIGN:
        return 1;
    default:
        return 0;
    }
}

std::unique_ptr<CallExprNode> Parser::parseCall(
    std::unique_ptr<ExprNode> callee)
{
    const Token& lparen = current();
    if (lparen.getType() != Token::LPAREN)
    {
        return nullptr; // error
    }
    next();
    std::vector<std::unique_ptr<ArgNode>> args;
    if (current().getType() != Token::RPAREN)
    {
        while (true)
        {
            args.emplace_back(parseCallArg());
            if (current().getType() != Token::COMMA)
            {
                break;
            }
            next();
        }
    }
    if (current().getType() != Token::RPAREN)
    {
        return nullptr; // error
    }
    next();
    return std::make_unique<CallExprNode>(std::move(callee),
        std::move(args), lparen.getPos());
}

std::unique_ptr<ArgNode> Parser::parseCallArg()
{
    const Token& identifier = current();
    if (identifier.getType() != Token::IDENTIFIER)
    {
        return nullptr; // error
    }
    std::string name = static_cast<const NameToken&>(identifier).getName();
    next();
    if (current().getType() != Token::COLON)
    {
        return nullptr; // error
    }
    next();
    return std::make_unique<ArgNode>(std::move(name), parseExpr(),
        identifier.getPos());
}

std::unique_ptr<BlockNode> Parser::parseBlock()
{
    const Token& lbrace = current();
    if (lbrace.getType() != Token::LBRACE)
    {
        return nullptr; // error
    }
    next();
    std::vector<std::unique_ptr<Node>> statements = parseStatements();
    if (current().getType() != Token::RBRACE)
    {
        return nullptr; // error
    }
    next();
    return std::make_unique<BlockNode>(std::move(statements), lbrace.getPos());
}
