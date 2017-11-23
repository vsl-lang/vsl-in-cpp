#include "parser/vslparser.hpp"

VSLParser::VSLParser(VSLContext& vslContext, Lexer& lexer, std::ostream& errors)
    : vslContext{ vslContext }, lexer{ lexer }, errors{ errors },
    errored{ false }
{
}

// program -> statements eof
BlockNode* VSLParser::parse()
{
    // it's assumed that the token cache is empty, so calling next() should get
    //  the very first token from the lexer
    Location savedLocation = next().location;
    std::vector<Node*> statements = parseStatements();
    if (current().kind != TokenKind::END)
    {
        errorExpected("eof");
    }
    return makeNode<BlockNode>(std::move(statements), savedLocation);
}

bool VSLParser::hasError() const
{
    return errored;
}

const Token& VSLParser::next()
{
    cache.emplace_back(lexer.nextToken());
    return current();
}

const Token& VSLParser::current()
{
    return cache.back();
}

EmptyNode* VSLParser::errorExpected(const char* s)
{
    const Token& t = current();
    errors << t.location << ": error: expected " << s << " but found " <<
        tokenKindName(t.kind) << '\n';
    errored = true;
    return makeNode<EmptyNode>(t.location);
}

EmptyNode* VSLParser::errorUnexpected(const Token& token)
{
    errors << token.location << ": error: unexpected token " <<
        tokenKindName(token.kind) << '\n';
    errored = true;
    return makeNode<EmptyNode>(token.location);
}

// statements -> statement*
std::vector<Node*> VSLParser::parseStatements()
{
    std::vector<Node*> statements;
    while (true)
    {
        // the cases here are in the follow set, telling when to stop expanding
        //  the statements production
        switch (current().kind)
        {
        case TokenKind::RBRACE:
        case TokenKind::END:
            return statements;
        default:
            statements.emplace_back(parseStatement());
        }
    }
}

// statement -> assignment | function | return | conditional | expr semicolon
//            | block | empty
Node* VSLParser::parseStatement()
{
    // the cases here are first sets, distinguishing which production to go for
    //  based on a token of lookahead
    switch (current().kind)
    {
    case TokenKind::KW_VAR:
    case TokenKind::KW_LET:
        return parseVariable();
    case TokenKind::KW_FUNC:
        return parseFunction();
    case TokenKind::KW_RETURN:
        return parseReturn();
    case TokenKind::KW_IF:
        return parseIf();
    case TokenKind::IDENTIFIER:
    case TokenKind::NUMBER:
    case TokenKind::KW_TRUE:
    case TokenKind::KW_FALSE:
    case TokenKind::PLUS:
    case TokenKind::MINUS:
    case TokenKind::LPAREN:
        {
            ExprNode* expr = parseExpr();
            if (current().kind != TokenKind::SEMICOLON)
            {
                return errorExpected("';'");
            }
            next();
            return expr;
        }
    case TokenKind::LBRACE:
        return parseBlock();
    case TokenKind::SEMICOLON:
        return parseEmptyStatement();
    default:
        ;
    }
    EmptyNode* e = errorUnexpected(current());
    next();
    return e;
}

// empty -> semicolon
EmptyNode* VSLParser::parseEmptyStatement()
{
    const Token& semicolon = current();
    if (semicolon.kind != TokenKind::SEMICOLON)
    {
        return errorExpected("';'");
    }
    next();
    return makeNode<EmptyNode>(semicolon.location);
}

// block -> lbrace statements rbrace
Node* VSLParser::parseBlock()
{
    const Token& lbrace = current();
    if (lbrace.kind != TokenKind::LBRACE)
    {
        return errorExpected("'{'");
    }
    next();
    std::vector<Node*> statements = parseStatements();
    if (current().kind != TokenKind::RBRACE)
    {
        return errorExpected("'}'");
    }
    next();
    return makeNode<BlockNode>(std::move(statements), lbrace.location);
}

// conditional -> if lparen expr rparen statement (else statement)?
Node* VSLParser::parseIf()
{
    Location location = current().location;
    if (current().kind != TokenKind::KW_IF)
    {
        return errorExpected("'if'");
    }
    if (next().kind != TokenKind::LPAREN)
    {
        return errorExpected("'('");
    }
    next();
    ExprNode* condition = parseExpr();
    if (current().kind != TokenKind::RPAREN)
    {
        return errorExpected("')'");
    }
    next();
    Node* thenCase = parseStatement();
    Node* elseCase;
    if (current().kind == TokenKind::KW_ELSE)
    {
        next();
        elseCase = parseStatement();
    }
    else
    {
        elseCase = makeNode<EmptyNode>(current().location);
    }
    return makeNode<IfNode>(condition, thenCase, elseCase, location);
}

// assignment -> (var | let) identifier colon type assign expr semicolon
Node* VSLParser::parseVariable()
{
    bool isConst;
    TokenKind k = current().kind;
    if (k == TokenKind::KW_VAR)
    {
        isConst = false;
    }
    else if (k == TokenKind::KW_LET)
    {
        isConst = true;
    }
    else
    {
        return errorExpected("'let' or 'var'");
    }
    Location location = current().location;
    next();
    const Token& id = current();
    if (id.kind != TokenKind::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    next();
    if (current().kind != TokenKind::COLON)
    {
        return errorExpected("':'");
    }
    next();
    const Type* type = parseType();
    if (current().kind != TokenKind::ASSIGN)
    {
        return errorExpected("'='");
    }
    next();
    ExprNode* value = parseExpr();
    if (current().kind != TokenKind::SEMICOLON)
    {
        return errorExpected("';'");
    }
    next();
    return makeNode<VariableNode>(id.text, type, value, isConst, location);
}

// function -> func identifier lparen param (comma param)* arrow type block
Node* VSLParser::parseFunction()
{
    const Token& func = current();
    if (func.kind != TokenKind::KW_FUNC)
    {
        return errorExpected("'func'");
    }
    const Location& location = func.location;
    const Token& id = next();
    if (id.kind != TokenKind::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    llvm::StringRef name = id.text;
    if (next().kind != TokenKind::LPAREN)
    {
        return errorExpected("'('");
    }
    next();
    std::vector<ParamNode*> params;
    if (current().kind != TokenKind::RPAREN)
    {
        while (true)
        {
            params.emplace_back(parseParam());
            if (current().kind != TokenKind::COMMA)
            {
                break;
            }
            next();
        }
    }
    if (current().kind != TokenKind::RPAREN)
    {
        return errorExpected("')'");
    }
    if (next().kind != TokenKind::ARROW)
    {
        return errorExpected("'->'");
    }
    next();
    const Type* returnType = parseType();
    Node* body = parseBlock();
    return makeNode<FunctionNode>(name, std::move(params), returnType, body,
        location);
}

// param -> identifier ':' type
ParamNode* VSLParser::parseParam()
{
    const Token& id = current();
    const Location& location = id.location;
    llvm::StringRef name;
    if (id.kind != TokenKind::IDENTIFIER)
    {
        errorExpected("identifier");
        name = "";
    }
    else
    {
        name = id.text;
    }
    if (next().kind != TokenKind::COLON)
    {
        errorExpected("':'");
    }
    else
    {
        next();
    }
    const Type* type = parseType();
    return makeNode<ParamNode>(name, type, location);
}

// return -> 'return' expr ';'
Node* VSLParser::parseReturn()
{
    const Token& ret = current();
    if (ret.kind != TokenKind::KW_RETURN)
    {
        return errorExpected("'return'");
    }
    const Location& location = ret.location;
    next();
    ExprNode* value = parseExpr();
    if (current().kind != TokenKind::SEMICOLON)
    {
        return errorExpected("';'");
    }
    next();
    return makeNode<ReturnNode>(value, location);
}

// Pratt parsing/TDOP (Top Down Operator Precedence) is used for expressions
ExprNode* VSLParser::parseExpr(int rbp)
{
    ExprNode* left = parseNud();
    while (rbp < getLbp(current()))
    {
        left = parseLed(left);
    }
    return left;
}

ExprNode* VSLParser::parseNud()
{
    const Token& token = current();
    const Location& location = token.location;
    next();
    switch (token.kind)
    {
    case TokenKind::IDENTIFIER:
        return makeNode<IdentNode>(token.text, location);
    case TokenKind::NUMBER:
        return parseNumber(token);
    case TokenKind::KW_TRUE:
        return makeNode<LiteralNode>(llvm::APInt{ 1, 1 }, location);
    case TokenKind::KW_FALSE:
        return makeNode<LiteralNode>(llvm::APInt{ 1, 0 }, location);
    case TokenKind::MINUS:
        return makeNode<UnaryNode>(token.kind, parseExpr(100), location);
    case TokenKind::LPAREN:
        {
            ExprNode* expr = parseExpr();
            if (current().kind != TokenKind::RPAREN)
            {
                errorExpected("')'");
            }
            next();
            return expr;
        }
    default:
        errorExpected("unary operator, identifier, or number");
        return makeNode<LiteralNode>(llvm::APInt{ 32, 0 }, location);
    }
}

ExprNode* VSLParser::parseLed(ExprNode* left)
{
    const Token& token = current();
    const Location& location = token.location;
    TokenKind k = token.kind;
    switch (k)
    {
    case TokenKind::PLUS:
    case TokenKind::MINUS:
    case TokenKind::STAR:
    case TokenKind::SLASH:
    case TokenKind::PERCENT:
    case TokenKind::EQUAL:
    case TokenKind::NOT_EQUAL:
    case TokenKind::GREATER:
    case TokenKind::GREATER_EQUAL:
    case TokenKind::LESS:
    case TokenKind::LESS_EQUAL:
        // left associative
        next();
        return makeNode<BinaryNode>(k, left, parseExpr(getLbp(token)),
            location);
    case TokenKind::ASSIGN:
        // right associative
        next();
        return makeNode<BinaryNode>(k, left, parseExpr(getLbp(token) - 1),
            location);
    case TokenKind::LPAREN:
        return parseCall(left);
    default:
        errorExpected("binary operator");
        return left;
    }
}

// lbp basically means operator precedence
// higher numbers mean higher precedence
int VSLParser::getLbp(const Token& token) const
{
    switch (token.kind)
    {
    case TokenKind::LPAREN:
        return 6;
    case TokenKind::STAR:
    case TokenKind::SLASH:
    case TokenKind::PERCENT:
        return 5;
    case TokenKind::PLUS:
    case TokenKind::MINUS:
        return 4;
    case TokenKind::GREATER:
    case TokenKind::GREATER_EQUAL:
    case TokenKind::LESS:
    case TokenKind::LESS_EQUAL:
        return 3;
    case TokenKind::EQUAL:
    case TokenKind::NOT_EQUAL:
        return 2;
    case TokenKind::ASSIGN:
        return 1;
    default:
        return 0;
    }
}

// call -> callee lparen arg (comma arg)* rparen
// callee is passed as an argument from parseLed()
CallNode* VSLParser::parseCall(ExprNode* callee)
{
    const Token& lparen = current();
    const Location& location = lparen.location;
    if (lparen.kind != TokenKind::LPAREN)
    {
        errorExpected("'('");
    }
    next();
    std::vector<ArgNode*> args;
    if (current().kind != TokenKind::RPAREN)
    {
        while (true)
        {
            args.emplace_back(parseCallArg());
            if (current().kind != TokenKind::COMMA)
            {
                break;
            }
            next();
        }
    }
    if (current().kind != TokenKind::RPAREN)
    {
        errorExpected("')'");
    }
    next();
    return makeNode<CallNode>(callee, args, location);
}

// arg -> identifier ':' expr
ArgNode* VSLParser::parseCallArg()
{
    const Token& id = current();
    const Location& location = id.location;
    llvm::StringRef name;
    if (id.kind != TokenKind::IDENTIFIER)
    {
        errorExpected("identifier");
        name = "";
    }
    else
    {
        name = id.text;
    }
    next();
    if (current().kind != TokenKind::COLON)
    {
        errorExpected("':'");
    }
    else
    {
        next();
    }
    ExprNode* value = parseExpr();
    return makeNode<ArgNode>(name, value, location);
}

LiteralNode* VSLParser::parseNumber(const Token& token)
{
    const Location& location = token.location;
    llvm::APInt value;
    if (token.text.getAsInteger(10, value))
    {
        errors << token.location << ": error: invalid integer '" <<
            token.text.str() << "'\n";
        value = 0;
        errored = true;
    }
    else if (value.getActiveBits() > 32)
    {
        errors << token.location << ": error: overflow detected in number '" <<
            token.text.str() << "'\n";
        errored = true;
    }
    value = value.zextOrTrunc(32);
    return makeNode<LiteralNode>(std::move(value), location);
}

// type -> 'Bool' | 'Int' | 'Void'
const Type* VSLParser::parseType()
{
    const Token& name = current();
    Type::Kind kind;
    switch (name.kind)
    {
    case TokenKind::KW_BOOL:
        kind = Type::BOOL;
        break;
    case TokenKind::KW_INT:
        kind = Type::INT;
        break;
    case TokenKind::KW_VOID:
        kind = Type::VOID;
        break;
    default:
        errorExpected("a type name");
        kind = Type::ERROR;
    }
    next();
    return vslContext.getSimpleType(kind);
}
