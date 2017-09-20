#include "vslparser.hpp"

VSLParser::VSLParser(Lexer& lexer, std::ostream& errors)
    : lexer{ lexer }, errors{ errors }, errored{ false }
{
}

std::unique_ptr<Node> VSLParser::parse()
{
    Location savedLocation = current().location;
    auto statements = parseStatements();
    if (current().kind != Token::SYMBOL_EOF)
    {
        return errorExpected("eof");
    }
    return std::make_unique<BlockNode>(std::move(statements), savedLocation);
}

bool VSLParser::hasError() const
{
    return errored;
}

const Token& VSLParser::next()
{
    cache.pop_front();
    return current();
}

const Token& VSLParser::current()
{
    return peek(0);
}

const Token& VSLParser::peek(size_t i)
{
    if (cache.size() > i)
    {
        return *cache[i];
    }
    else
    {
        size_t j = i - cache.size();
        for (size_t k = 0; k <= j; ++k)
        {
            cache.emplace_back(lexer.nextToken());
        }
        return *cache.back();
    }
}

std::unique_ptr<Node> VSLParser::errorExpected(const char* s)
{
    const Token& t = current();
    errors << t.location << ": error: expected " << s << " but found " <<
        Token::kindToString(t.kind) << '\n';
    errored = true;
    return std::make_unique<ErrorNode>(t.location);
}

std::unique_ptr<Node> VSLParser::errorUnexpected(const Token& token)
{
    errors << token.location << ": error: unexpected token " <<
        Token::kindToString(token.kind) << '\n';
    errored = true;
    return std::make_unique<ErrorNode>(token.location);
}

std::vector<std::unique_ptr<Node>> VSLParser::parseStatements()
{
    std::vector<std::unique_ptr<Node>> statements;
    while (true)
    {
        switch (current().kind)
        {
        case Token::SYMBOL_RBRACE:
        case Token::SYMBOL_EOF:
            return statements;
        default:
            statements.emplace_back(parseStatement());
        }
    }
}

std::unique_ptr<Node> VSLParser::parseStatement()
{
    switch (current().kind)
    {
    case Token::KEYWORD_VAR:
    case Token::KEYWORD_LET:
        return parseAssignment();
    case Token::KEYWORD_FUNC:
        return parseFunction();
    case Token::KEYWORD_RETURN:
        return parseReturn();
    case Token::KEYWORD_IF:
        return parseConditional();
    case Token::IDENTIFIER:
    case Token::NUMBER:
    case Token::OP_PLUS:
    case Token::OP_MINUS:
    case Token::SYMBOL_LPAREN:
        {
            std::unique_ptr<Node> expr = parseExpr();
            if (current().kind != Token::SYMBOL_SEMICOLON)
            {
                return errorExpected("';'");
            }
            next();
            return expr;
        }
    case Token::SYMBOL_LBRACE:
        return parseBlock();
    case Token::SYMBOL_SEMICOLON:
        return parseEmptyStatement();
    default:
        ;
    }
    auto e = errorUnexpected(current());
    next();
    return e;
}

std::unique_ptr<Node> VSLParser::parseEmptyStatement()
{
    const Token& semicolon = current();
    if (semicolon.kind != Token::SYMBOL_SEMICOLON)
    {
        return errorExpected("';'");
    }
    next();
    return std::make_unique<EmptyNode>(semicolon.location);
}

std::unique_ptr<Node> VSLParser::parseBlock()
{
    const Token& lbrace = current();
    if (lbrace.kind != Token::SYMBOL_LBRACE)
    {
        return errorExpected("'{'");
    }
    next();
    std::vector<std::unique_ptr<Node>> statements = parseStatements();
    if (current().kind != Token::SYMBOL_RBRACE)
    {
        return errorExpected("'}'");
    }
    next();
    return std::make_unique<BlockNode>(std::move(statements), lbrace.location);
}

std::unique_ptr<Node> VSLParser::parseConditional()
{
    Location savedLocation = current().location;
    if (current().kind != Token::KEYWORD_IF)
    {
        return errorExpected("'if'");
    }
    if (next().kind != Token::SYMBOL_LPAREN)
    {
        return errorExpected("'('");
    }
    next();
    std::unique_ptr<Node> condition = parseExpr();
    if (current().kind != Token::SYMBOL_RPAREN)
    {
        return errorExpected("')'");
    }
    next();
    std::unique_ptr<Node> thenCase = parseStatement();
    std::unique_ptr<Node> elseCase;
    if (current().kind == Token::KEYWORD_ELSE)
    {
        next();
        elseCase = parseStatement();
    }
    else
    {
        elseCase = std::make_unique<EmptyNode>(current().location);
    }
    return std::make_unique<ConditionalNode>(std::move(condition),
        std::move(thenCase), std::move(elseCase), savedLocation);
}

std::unique_ptr<Node> VSLParser::parseAssignment()
{
    AssignmentNode::Qualifiers qualifiers;
    Token::Kind k = current().kind;
    Location savedLocation = current().location;
    if (k == Token::KEYWORD_VAR)
    {
        qualifiers = AssignmentNode::NONCONST;
    }
    else if (k == Token::KEYWORD_LET)
    {
        qualifiers = AssignmentNode::CONST;
    }
    else
    {
        return errorExpected("'let' or 'var'");
    }
    next();
    const Token& id = current();
    if (id.kind != Token::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    std::string name = static_cast<const NameToken&>(id).name;
    next();
    if (current().kind != Token::SYMBOL_COLON)
    {
        return errorExpected("':'");
    }
    next();
    std::unique_ptr<Type> type = parseType();
    if (current().kind != Token::OP_ASSIGN)
    {
        return errorExpected("'='");
    }
    next();
    std::unique_ptr<Node> value = parseExpr();
    if (current().kind != Token::SYMBOL_SEMICOLON)
    {
        return errorExpected("';'");
    }
    next();
    return std::make_unique<AssignmentNode>(std::move(name),
        std::move(type), std::move(value), qualifiers, savedLocation);
}

std::unique_ptr<Node> VSLParser::parseFunction()
{
    const Token& func = current();
    if (func.kind != Token::KEYWORD_FUNC)
    {
        return errorExpected("'func'");
    }
    Location savedLocation = func.location;
    const Token& id = next();
    if (id.kind != Token::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    std::string name = static_cast<const NameToken&>(id).name;
    if (next().kind != Token::SYMBOL_LPAREN)
    {
        return errorExpected("'('");
    }
    next();
    std::vector<FunctionNode::Param> params;
    if (current().kind != Token::SYMBOL_RPAREN)
    {
        while (true)
        {
            params.emplace_back(parseParam());
            if (current().kind != Token::SYMBOL_COMMA)
            {
                break;
            }
            next();
        }
    }
    if (current().kind != Token::SYMBOL_RPAREN)
    {
        return errorExpected("')'");
    }
    if (next().kind != Token::SYMBOL_ARROW)
    {
        return errorExpected("'->'");
    }
    next();
    std::unique_ptr<Type> returnType = parseType();
    std::unique_ptr<Node> body = parseBlock();
    return std::make_unique<FunctionNode>(std::move(name), std::move(params),
        std::move(returnType), std::move(body), savedLocation);
}

std::unique_ptr<Node> VSLParser::parseReturn()
{
    const Token& ret = current();
    if (ret.kind != Token::KEYWORD_RETURN)
    {
        return errorExpected("'return'");
    }
    Location savedLocation = ret.location;
    next();
    std::unique_ptr<Node> value = parseExpr();
    if (current().kind != Token::SYMBOL_SEMICOLON)
    {
        return errorExpected("';'");
    }
    next();
    return std::make_unique<ReturnNode>(std::move(value), savedLocation);
}

FunctionNode::Param VSLParser::parseParam()
{
    const Token& id = current();
    std::string str;
    if (id.kind != Token::IDENTIFIER)
    {
        errorExpected("identifier");
        str = "";
    }
    else
    {
        str = static_cast<const NameToken&>(id).name;
    }
    FunctionNode::ParamName name{ std::move(str), id.location };
    if (next().kind != Token::SYMBOL_COLON)
    {
        errorExpected("':'");
    }
    next();
    std::unique_ptr<Type> type = parseType();
    return { std::move(name), std::move(type) };
}

std::unique_ptr<Type> VSLParser::parseType()
{
    const Token& name = current();
    Type::Kind kind;
    if (name.kind == Token::KEYWORD_INT)
    {
        kind = Type::INT;
    }
    else if (name.kind == Token::KEYWORD_VOID)
    {
        kind = Type::VOID;
    }
    else
    {
        errorExpected("a type name");
        kind = Type::ERROR;
    }
    next();
    return std::make_unique<SimpleType>(kind);
}

std::unique_ptr<Node> VSLParser::parseExpr(int rbp)
{
    // top down operator precedence algorithm
    std::unique_ptr<Node> left = parseNud();
    while (rbp < getLbp(current()))
    {
        left = parseLed(std::move(left));
    }
    return left;
}

std::unique_ptr<Node> VSLParser::parseNud()
{
    const Token& token = current();
    next();
    switch (token.kind)
    {
    case Token::IDENTIFIER:
        return std::make_unique<IdentExprNode>(
            static_cast<const NameToken&>(token).name, token.location);
    case Token::NUMBER:
        return std::make_unique<NumberExprNode>(
            static_cast<const NumberToken&>(token).value,
            token.location);
    case Token::OP_PLUS:
    case Token::OP_MINUS:
        return std::make_unique<UnaryExprNode>(token.kind, parseExpr(100),
            token.location);
    default:
        return errorExpected("unary operator, identifier, or number");
    }
}

std::unique_ptr<Node> VSLParser::parseLed(std::unique_ptr<Node> left)
{
    const Token& token = current();
    Token::Kind k = token.kind;
    switch (k)
    {
    case Token::OP_STAR:
    case Token::OP_SLASH:
    case Token::OP_PERCENT:
    case Token::OP_PLUS:
    case Token::OP_MINUS:
    case Token::OP_GREATER:
    case Token::OP_GREATER_EQUAL:
    case Token::OP_LESS:
    case Token::OP_LESS_EQUAL:
    case Token::OP_EQUALS:
        next();
        return std::make_unique<BinaryExprNode>(k, std::move(left),
            parseExpr(getLbp(token)), token.location);
    case Token::OP_ASSIGN:
        next();
        return std::make_unique<BinaryExprNode>(k, std::move(left),
            parseExpr(getLbp(token) - 1), token.location);
    case Token::SYMBOL_LPAREN:
        return parseCall(std::move(left));
    default:
        return errorExpected("binary operator");
    }
}

int VSLParser::getLbp(const Token& token) const
{
    switch (token.kind)
    {
    case Token::SYMBOL_LPAREN:
        return 6;
    case Token::OP_STAR:
    case Token::OP_SLASH:
    case Token::OP_PERCENT:
        return 5;
    case Token::OP_PLUS:
    case Token::OP_MINUS:
        return 4;
    case Token::OP_GREATER:
    case Token::OP_GREATER_EQUAL:
    case Token::OP_LESS:
    case Token::OP_LESS_EQUAL:
        return 3;
    case Token::OP_EQUALS:
        return 2;
    case Token::OP_ASSIGN:
        return 1;
    default:
        return 0;
    }
}

std::unique_ptr<Node> VSLParser::parseCall(
    std::unique_ptr<Node> callee)
{
    const Token& lparen = current();
    if (lparen.kind != Token::SYMBOL_LPAREN)
    {
        return errorExpected("'('");
    }
    next();
    std::vector<std::unique_ptr<Node>> args;
    if (current().kind != Token::SYMBOL_RPAREN)
    {
        while (true)
        {
            args.emplace_back(parseCallArg());
            if (current().kind != Token::SYMBOL_COMMA)
            {
                break;
            }
            next();
        }
    }
    if (current().kind != Token::SYMBOL_RPAREN)
    {
        return errorExpected("')'");
    }
    next();
    return std::make_unique<CallExprNode>(std::move(callee),
        std::move(args), lparen.location);
}

std::unique_ptr<Node> VSLParser::parseCallArg()
{
    const Token& identifier = current();
    if (identifier.kind != Token::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    std::string name = static_cast<const NameToken&>(identifier).name;
    next();
    if (current().kind != Token::SYMBOL_COLON)
    {
        return errorExpected("':'");
    }
    next();
    return std::make_unique<ArgNode>(std::move(name), parseExpr(),
        identifier.location);
}
