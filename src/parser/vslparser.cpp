#include "parser/vslparser.hpp"

VSLParser::VSLParser(VSLContext& vslContext, Lexer& lexer)
    : vslContext{ vslContext }, lexer{ lexer }
{
}

// program -> statements eof
BlockNode* VSLParser::parse()
{
    Location location = current().location;
    std::vector<Node*> statements = parseStatements();
    if (current().kind != TokenKind::END)
    {
        errorExpected("eof");
    }
    return makeNode<BlockNode>(std::move(statements), location);
}

Token VSLParser::consume()
{
    if (cache.empty())
    {
        // shortcut: take a token directly from the lexer
        return lexer.nextToken();
    }
    // remove a token from the cache
    Token t = std::move(cache.front());
    cache.pop_front();
    return t;
}

const Token& VSLParser::current()
{
    return peek(0);
}

const Token& VSLParser::peek(size_t depth)
{
    if (depth >= cache.size())
    {
        // needs (depth-cache.size()+1) more tokens
        for (size_t i = cache.size(); i <= depth; ++i)
        {
            cache.emplace_back(lexer.nextToken());
        }
    }
    return cache[depth];
}

EmptyNode* VSLParser::errorExpected(const char* s)
{
    const Token& t = current();
    vslContext.error(t.location) << "expected " << s << " but found " <<
        tokenKindName(t.kind) << '\n';
    return makeNode<EmptyNode>(t.location);
}

EmptyNode* VSLParser::errorUnexpected(const Token& token)
{
    vslContext.error(token.location) << "unexpected token " <<
        tokenKindName(token.kind) << '\n';
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
            consume();
            return expr;
        }
    case TokenKind::LBRACE:
        return parseBlock();
    case TokenKind::SEMICOLON:
        return parseEmptyStatement();
    default:
        EmptyNode* e = errorUnexpected(consume());
        return e;
    }
}

// empty -> semicolon
EmptyNode* VSLParser::parseEmptyStatement()
{
    if (current().kind != TokenKind::SEMICOLON)
    {
        return errorExpected("';'");
    }
    Location location = consume().location;
    return makeNode<EmptyNode>(location);
}

// block -> lbrace statements rbrace
Node* VSLParser::parseBlock()
{
    if (current().kind != TokenKind::LBRACE)
    {
        return errorExpected("'{'");
    }
    Location location = consume().location;
    std::vector<Node*> statements = parseStatements();
    if (current().kind != TokenKind::RBRACE)
    {
        return errorExpected("'}'");
    }
    consume();
    return makeNode<BlockNode>(std::move(statements), location);
}

// conditional -> if lparen expr rparen statement (else statement)?
Node* VSLParser::parseIf()
{
    if (current().kind != TokenKind::KW_IF)
    {
        return errorExpected("'if'");
    }
    Location location = consume().location;
    if (current().kind != TokenKind::LPAREN)
    {
        return errorExpected("'('");
    }
    consume();
    ExprNode* condition = parseExpr();
    if (current().kind != TokenKind::RPAREN)
    {
        return errorExpected("')'");
    }
    consume();
    Node* thenCase = parseStatement();
    Node* elseCase;
    if (current().kind == TokenKind::KW_ELSE)
    {
        consume();
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
    Location location = consume().location;
    if (current().kind != TokenKind::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    llvm::StringRef name = consume().text;
    if (current().kind != TokenKind::COLON)
    {
        return errorExpected("':'");
    }
    consume();
    const Type* type = parseType();
    if (current().kind != TokenKind::ASSIGN)
    {
        return errorExpected("'='");
    }
    consume();
    ExprNode* value = parseExpr();
    if (current().kind != TokenKind::SEMICOLON)
    {
        return errorExpected("';'");
    }
    consume();
    return makeNode<VariableNode>(name, type, value, isConst, location);
}

// function -> func identifier lparen param (comma param)* arrow type block
Node* VSLParser::parseFunction()
{
    if (current().kind != TokenKind::KW_FUNC)
    {
        return errorExpected("'func'");
    }
    Location location = consume().location;
    if (current().kind != TokenKind::IDENTIFIER)
    {
        return errorExpected("identifier");
    }
    llvm::StringRef name = consume().text;
    if (current().kind != TokenKind::LPAREN)
    {
        return errorExpected("'('");
    }
    consume();
    std::vector<ParamNode*> params;
    if (current().kind != TokenKind::RPAREN)
    {
        while (true)
        {
            ParamNode* param = parseParam();
            if (param->type != vslContext.getVoidType())
            {
                params.push_back(param);
            }
            else
            {
                vslContext.error(param->location) << "type " <<
                    param->type->toString() << " is invalid for parameter " <<
                    param->name << '\n';
                // FIXME: delete the invalid ParamNode
            }
            if (current().kind != TokenKind::COMMA)
            {
                break;
            }
            consume();
        }
    }
    if (current().kind != TokenKind::RPAREN)
    {
        return errorExpected("')'");
    }
    consume();
    if (current().kind != TokenKind::ARROW)
    {
        return errorExpected("'->'");
    }
    consume();
    const Type* returnType = parseType();
    Node* body = parseBlock();
    std::vector<const Type*> paramTypes;
    paramTypes.resize(params.size());
    std::transform(params.begin(), params.end(), paramTypes.begin(),
        [](ParamNode* p)
        {
            return p->type;
        });
    const FunctionType* ft = vslContext.getFunctionType(std::move(paramTypes),
        returnType);
    return makeNode<FunctionNode>(name, std::move(params), returnType, body, ft,
        location);
}

// param -> identifier ':' type
ParamNode* VSLParser::parseParam()
{
    Location location = current().location;
    llvm::StringRef name;
    if (current().kind != TokenKind::IDENTIFIER)
    {
        errorExpected("identifier");
        name = "";
        consume();
    }
    else
    {
        name = consume().text;
    }
    if (consume().kind != TokenKind::COLON)
    {
        // consumed here because the missing colon was likely a typo
        errorExpected("':'");
    }
    const Type* type = parseType();
    return makeNode<ParamNode>(name, type, location);
}

// return -> 'return' expr ';'
Node* VSLParser::parseReturn()
{
    if (current().kind != TokenKind::KW_RETURN)
    {
        return errorExpected("'return'");
    }
    Location location = consume().location;
    ExprNode* value;
    if (current().kind == TokenKind::SEMICOLON)
    {
        // return without a value (void)
        consume();
        value = nullptr;
    }
    else
    {
        // return with a value
        value = parseExpr();
        if (current().kind != TokenKind::SEMICOLON)
        {
            return errorExpected("';'");
        }
        consume();
    }
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
    Token t = consume();
    switch (t.kind)
    {
    case TokenKind::IDENTIFIER:
        return makeNode<IdentNode>(t.text, t.location);
    case TokenKind::NUMBER:
        return parseNumber(t);
    case TokenKind::KW_TRUE:
        return makeNode<LiteralNode>(llvm::APInt{ 1, 1 }, t.location);
    case TokenKind::KW_FALSE:
        return makeNode<LiteralNode>(llvm::APInt{ 1, 0 }, t.location);
    case TokenKind::MINUS:
        return makeNode<UnaryNode>(t.kind, parseExpr(100), t.location);
    case TokenKind::LPAREN:
        {
            ExprNode* expr = parseExpr();
            if (current().kind != TokenKind::RPAREN)
            {
                errorExpected("')'");
            }
            consume();
            return expr;
        }
    default:
        errorExpected("unary operator, identifier, or number");
        return makeNode<LiteralNode>(llvm::APInt{ 32, 0 }, t.location);
    }
}

ExprNode* VSLParser::parseLed(ExprNode* left)
{
    const Token& t = current();
    switch (t.kind)
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
        consume();
        return makeNode<BinaryNode>(t.kind, left, parseExpr(getLbp(t)),
            t.location);
    case TokenKind::ASSIGN:
        // right associative
        consume();
        return makeNode<BinaryNode>(t.kind, left, parseExpr(getLbp(t) - 1),
            t.location);
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
    if (current().kind != TokenKind::LPAREN)
    {
        errorExpected("'('");
    }
    Location location = consume().location;
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
            consume();
        }
    }
    if (current().kind != TokenKind::RPAREN)
    {
        errorExpected("')'");
    }
    consume();
    return makeNode<CallNode>(callee, args, location);
}

// arg -> identifier ':' expr
ArgNode* VSLParser::parseCallArg()
{
    llvm::StringRef name;
    if (current().kind != TokenKind::IDENTIFIER)
    {
        errorExpected("identifier");
        name = "";
    }
    else
    {
        name = current().text;
    }
    Location location = consume().location;
    if (consume().kind != TokenKind::COLON)
    {
        // consumed here because a missing colon is most likely a typo
        errorExpected("':'");
    }
    ExprNode* value = parseExpr();
    return makeNode<ArgNode>(name, value, location);
}

LiteralNode* VSLParser::parseNumber(const Token& token)
{
    const Location& location = token.location;
    // make sure that the text inside is a valid 32bit integer
    llvm::APInt value;
    if (token.text.getAsInteger(10, value))
    {
        vslContext.error(location) << "invalid integer '" << token.text <<
            "'\n";
        value = 0;
    }
    else if (value.getActiveBits() > 32)
    {
        vslContext.error(location) << "overflow detected in number '" <<
            token.text<< "'\n";
    }
    value = value.zextOrTrunc(32);
    return makeNode<LiteralNode>(std::move(value), location);
}

// type -> 'Bool' | 'Int' | 'Void'
const Type* VSLParser::parseType()
{
    switch (consume().kind)
    {
    case TokenKind::KW_BOOL:
        return vslContext.getBoolType();
    case TokenKind::KW_INT:
        return vslContext.getIntType();
    case TokenKind::KW_VOID:
        return vslContext.getVoidType();
    default:
        errorExpected("a type name");
        return vslContext.getErrorType();
    }
}
