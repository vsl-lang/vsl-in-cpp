#include "parser.hpp"

Parser::Parser(std::vector<std::unique_ptr<Token>> tokens, std::ostream& errors)
    : tokens{ std::move(tokens) }, pos{ this->tokens.begin() }, errors{ errors }
{
}

std::unique_ptr<Node> Parser::parse()
{
    Location savedLocation = current().getLocation();
    auto root = std::make_unique<BlockNode>(parseStatements(),
        savedLocation);
    if (current().getType() != Token::END)
    {
        return errorExpected("eof");
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

std::unique_ptr<Node> Parser::errorExpected(const char* s)
{
    const Token& t = current();
    Location location = t.getLocation();
    errors << location << ": error: expected " << s << " but found " <<
        Token::typeToString(t.getType()) << '\n';
    return std::make_unique<ErrorNode>(location);
}

std::unique_ptr<Node> Parser::errorUnexpected(const Token& token)
{
    errors << token.getLocation() << ": error: unexpected token " <<
        Token::typeToString(token.getType()) << '\n';
    return std::make_unique<ErrorNode>(token.getLocation());
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
    case Token::VAR:
    case Token::LET:
        return parseAssignment();
    case Token::FUNC:
        return parseFunction();
    case Token::RETURN:
        return parseReturn();
    case Token::IF:
        return parseConditional();
    case Token::IDENTIFIER:
    case Token::NUMBER:
    case Token::PLUS:
    case Token::MINUS:
    case Token::LPAREN:
        {
            std::unique_ptr<Node> expr = parseExpr();
            if (current().getType() != Token::SEMICOLON)
            {
                return errorExpected("';'");
            }
            next();
            return expr;
        }
    case Token::LBRACE:
        return parseBlock();
    case Token::SEMICOLON:
        return parseEmptyStatement();
    default:
        ;
    }
    auto e = errorUnexpected(current());
    next();
    return e;
}

std::unique_ptr<Node> Parser::parseEmptyStatement()
{
    const Token& semicolon = current();
    if (semicolon.getType() != Token::SEMICOLON)
    {
        return errorExpected("';'");
    }
    next();
    return std::make_unique<EmptyNode>(semicolon.getLocation());
}

std::unique_ptr<Node> Parser::parseBlock()
{
    const Token& lbrace = current();
    if (lbrace.getType() != Token::LBRACE)
    {
        return errorExpected("'{'");
    }
    next();
    std::vector<std::unique_ptr<Node>> statements = parseStatements();
    if (current().getType() != Token::RBRACE)
    {
        return errorExpected("'}'");
    }
    next();
    return std::make_unique<BlockNode>(std::move(statements),
        lbrace.getLocation());
}

std::unique_ptr<Node> Parser::parseConditional()
{
    Location savedLocation = current().getLocation();
    if (current().getType() != Token::IF)
    {
        return errorExpected("'if'");
    }
    if (next().getType() != Token::LPAREN)
    {
        return errorExpected("'('");
    }
    next();
    std::unique_ptr<Node> condition = parseExpr();
    if (current().getType() != Token::RPAREN)
    {
        return errorExpected("')'");
    }
    next();
    std::unique_ptr<Node> thenCase = parseStatement();
    std::unique_ptr<Node> elseCase;
    if (current().getType() == Token::ELSE)
    {
        next();
        elseCase = parseStatement();
    }
    else
    {
        elseCase = std::make_unique<EmptyNode>(current().getLocation());
    }
    return std::make_unique<ConditionalNode>(std::move(condition),
        std::move(thenCase), std::move(elseCase), savedLocation);
}

std::unique_ptr<Node> Parser::parseAssignment()
{
    AssignmentNode::Qualifiers qualifiers;
    Token::Type t = current().getType();
    Location savedLocation = current().getLocation();
    if (t == Token::VAR)
    {
        qualifiers = AssignmentNode::NONCONST;
    }
    else if (t == Token::LET)
    {
        qualifiers = AssignmentNode::CONST;
    }
    else
    {
        return errorExpected("'let' or 'var'");
    }
    next();
    const Token& id = current();
    if (id.getType() != Token::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    std::string name = static_cast<const NameToken&>(id).getName();
    next();
    if (current().getType() != Token::COLON)
    {
        return errorExpected("':'");
    }
    next();
    if (current().getType() != Token::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    std::unique_ptr<Node> type = parseType();
    if (current().getType() != Token::ASSIGN)
    {
        return errorExpected("'='");
    }
    next();
    std::unique_ptr<Node> value = parseExpr();
    if (current().getType() != Token::SEMICOLON)
    {
        return errorExpected("';'");
    }
    next();
    return std::make_unique<AssignmentNode>(std::move(name),
        std::move(type), std::move(value), qualifiers, savedLocation);
}

std::unique_ptr<Node> Parser::parseFunction()
{
    const Token& func = current();
    if (func.getType() != Token::FUNC)
    {
        return errorExpected("'func'");
    }
    Location savedLocation = func.getLocation();
    const Token& id = next();
    if (id.getType() != Token::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    std::string name = static_cast<const NameToken&>(id).getName();
    if (next().getType() != Token::LPAREN)
    {
        return errorExpected("'('");
    }
    next();
    std::vector<std::unique_ptr<Node>> params;
    if (current().getType() != Token::RPAREN)
    {
        while (true)
        {
            params.emplace_back(parseParam());
            if (current().getType() != Token::COMMA)
            {
                break;
            }
            next();
        }
    }
    if (current().getType() != Token::RPAREN)
    {
        return errorExpected("')'");
    }
    if (next().getType() != Token::ARROW)
    {
        return errorExpected("'->'");
    }
    next();
    std::unique_ptr<Node> returnType = parseType();
    std::unique_ptr<Node> body = parseBlock();
    return std::make_unique<FunctionNode>(std::move(name), std::move(params),
        std::move(returnType), std::move(body), savedLocation);
}

std::unique_ptr<Node> Parser::parseReturn()
{
    const Token& ret = current();
    if (ret.getType() != Token::RETURN)
    {
        return errorExpected("'return'");
    }
    Location savedLocation = ret.getLocation();
    next();
    std::unique_ptr<Node> value = parseExpr();
    if (current().getType() != Token::SEMICOLON)
    {
        return errorExpected("';'");
    }
    next();
    return std::make_unique<ReturnNode>(std::move(value), savedLocation);
}

std::unique_ptr<Node> Parser::parseParam()
{
    const Token& id = current();
    if (id.getType() != Token::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    Location savedLocation = id.getLocation();
    std::string name = static_cast<const NameToken&>(id).getName();
    if (next().getType() != Token::COLON)
    {
        return errorExpected("':'");
    }
    next();
    std::unique_ptr<Node> type = parseType();
    return std::make_unique<ParamNode>(std::move(name), std::move(type),
        savedLocation);
}

std::unique_ptr<Node> Parser::parseType()
{
    const Token& id = current();
    if (id.getType() != Token::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    next();
    std::string name = static_cast<const NameToken&>(id).getName();
    return std::make_unique<TypeNode>(std::move(name), id.getLocation());
}

std::unique_ptr<Node> Parser::parseExpr(int rbp)
{
    // top down operator precedence algorithm
    std::unique_ptr<Node> left = parseNud();
    while (rbp < getLbp(current()))
    {
        left = parseLed(std::move(left));
    }
    return left;
}

std::unique_ptr<Node> Parser::parseNud()
{
    const Token& token = current();
    next();
    switch (token.getType())
    {
    case Token::IDENTIFIER:
        return std::make_unique<IdentExprNode>(static_cast<const NameToken&>(
                token).getName(), token.getLocation());
    case Token::NUMBER:
        return std::make_unique<NumberExprNode>(
            static_cast<const NumberToken&>(token).getValue(),
            token.getLocation());
    case Token::PLUS:
    case Token::MINUS:
        return std::make_unique<UnaryExprNode>(token.getType(), parseExpr(100),
            token.getLocation());
    default:
        return errorExpected("unary operator, identifier, or number");
    }
}

std::unique_ptr<Node> Parser::parseLed(std::unique_ptr<Node> left)
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
            parseExpr(getLbp(token)), token.getLocation());
    case Token::ASSIGN:
        next();
        return std::make_unique<BinaryExprNode>(type, std::move(left),
            parseExpr(getLbp(token) - 1), token.getLocation());
    case Token::LPAREN:
        return parseCall(std::move(left));
    default:
        return errorExpected("binary operator");
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

std::unique_ptr<Node> Parser::parseCall(
    std::unique_ptr<Node> callee)
{
    const Token& lparen = current();
    if (lparen.getType() != Token::LPAREN)
    {
        return errorExpected("'('");
    }
    next();
    std::vector<std::unique_ptr<Node>> args;
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
        return errorExpected("')'");
    }
    next();
    return std::make_unique<CallExprNode>(std::move(callee),
        std::move(args), lparen.getLocation());
}

std::unique_ptr<Node> Parser::parseCallArg()
{
    const Token& identifier = current();
    if (identifier.getType() != Token::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    std::string name = static_cast<const NameToken&>(identifier).getName();
    next();
    if (current().getType() != Token::COLON)
    {
        return errorExpected("':'");
    }
    next();
    return std::make_unique<ArgNode>(std::move(name), parseExpr(),
        identifier.getLocation());
}
