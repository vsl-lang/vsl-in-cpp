#include "parser/vslparser.hpp"

VSLParser::VSLParser(VSLContext& vslContext, Lexer& lexer)
    : vslContext{ vslContext }, lexer{ lexer }, diag{ lexer.getDiag() }
{
}

// program -> statements eof
std::vector<Node*> VSLParser::parse()
{
    std::vector<Node*> statements = parseGlobals();
    if (current().isNot(TokenKind::END))
    {
        errorExpected("eof");
    }
    return statements;
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
    // make sure enough tokens are already cached
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

bool VSLParser::empty() const
{
    return cache.empty() && lexer.empty();
}

EmptyNode* VSLParser::errorExpected(const char* s)
{
    const Token& token = current();
    diag.print<Diag::EXPECTED_BUT_FOUND>(s, token);
    return makeNode<EmptyNode>(token.getLoc());
}

EmptyNode* VSLParser::errorUnexpected(const Token& token)
{
    diag.print<Diag::UNEXPECTED_TOKEN>(token);
    return makeNode<EmptyNode>(token.getLoc());
}

// globals -> function*
std::vector<Node*> VSLParser::parseGlobals()
{
    std::vector<Node*> globals;
    while (!empty())
    {
        // only functions can be in the global scope
        switch (current().getKind())
        {
        case TokenKind::KW_FUNC:
            globals.emplace_back(parseFunction());
        case TokenKind::END:
            break;
        default:
            errorExpected("'func'");
            consume();
        }
        if (current().is(TokenKind::END))
        {
            break;
        }
    }
    return globals;
}

// statements -> statement*
std::vector<Node*> VSLParser::parseStatements()
{
    std::vector<Node*> statements;
    while (!empty())
    {
        // the cases here are in the follow set, telling when to stop expanding
        //  the statements production
        switch (current().getKind())
        {
        case TokenKind::RBRACE:
        case TokenKind::END:
            break;
        default:
            statements.push_back(parseStatement());
        }
        if (current().is(TokenKind::RBRACE) || current().is(TokenKind::END))
        {
            break;
        }
    }
    return statements;
}

// statement -> variable | return | conditional | exprstmt | block | empty
Node* VSLParser::parseStatement()
{
    // the cases here are first sets of an LL parser, distinguishing which
    //  production to go for based on a token of lookahead
    switch (current().getKind())
    {
    case TokenKind::KW_VAR:
    case TokenKind::KW_LET:
        return parseVariable();
    case TokenKind::KW_RETURN:
        return parseReturn();
    case TokenKind::KW_IF:
        return parseIf();
    case TokenKind::IDENTIFIER:
    case TokenKind::NUMBER:
    case TokenKind::KW_TRUE:
    case TokenKind::KW_FALSE:
    case TokenKind::MINUS:
    case TokenKind::LPAREN:
        return parseExprStmt();
    case TokenKind::LBRACE:
        return parseBlock();
    case TokenKind::KW_FUNC:
        // function within a function
        diag.print<Diag::FUNCEPTION>(current().getLoc());
        // fallthrough
    case TokenKind::SEMICOLON:
        return makeNode<EmptyNode>(consume().getLoc());
    default:
        EmptyNode* e = errorUnexpected(consume());
        return e;
    }
}

// block -> lbrace statements rbrace
Node* VSLParser::parseBlock()
{
    if (current().isNot(TokenKind::LBRACE))
    {
        return errorExpected("'{'");
    }
    Location location = consume().getLoc();
    std::vector<Node*> statements = parseStatements();
    if (current().isNot(TokenKind::RBRACE))
    {
        return errorExpected("'}'");
    }
    consume();
    return makeNode<BlockNode>(location, std::move(statements));
}

// conditional -> if lparen expr rparen statement (else statement)?
Node* VSLParser::parseIf()
{
    if (current().isNot(TokenKind::KW_IF))
    {
        return errorExpected("'if'");
    }
    Location location = consume().getLoc();
    if (current().isNot(TokenKind::LPAREN))
    {
        return errorExpected("'('");
    }
    consume();
    ExprNode* condition = parseExpr();
    if (current().isNot(TokenKind::RPAREN))
    {
        return errorExpected("')'");
    }
    consume();
    Node* thenCase = parseStatement();
    Node* elseCase;
    if (current().is(TokenKind::KW_ELSE))
    {
        consume();
        elseCase = parseStatement();
    }
    else
    {
        elseCase = makeNode<EmptyNode>(current().getLoc());
    }
    return makeNode<IfNode>(location, condition, thenCase, elseCase);
}

// assignment -> (var | let) identifier colon type assign expr semicolon
Node* VSLParser::parseVariable()
{
    bool constness;
    TokenKind k = current().getKind();
    if (k == TokenKind::KW_VAR)
    {
        constness = false;
    }
    else if (k == TokenKind::KW_LET)
    {
        constness = true;
    }
    else
    {
        return errorExpected("'let' or 'var'");
    }
    Location location = consume().getLoc();
    if (current().isNot(TokenKind::IDENTIFIER))
    {
        return errorExpected("identifier");
    }
    llvm::StringRef name = consume().getText();
    if (current().isNot(TokenKind::COLON))
    {
        return errorExpected("':'");
    }
    consume();
    const Type* type = parseType();
    if (current().isNot(TokenKind::ASSIGN))
    {
        return errorExpected("'='");
    }
    consume();
    ExprNode* value = parseExpr();
    if (current().isNot(TokenKind::SEMICOLON))
    {
        return errorExpected("';'");
    }
    consume();
    return makeNode<VariableNode>(location, name, type, value, constness);
}

// function -> funcInterface block | extfunc
// extfunc -> funcInterface external lparen ident rparen semicolon
// funcInterface -> func ident lparen param (comma param)* rparen arrow type
Node* VSLParser::parseFunction()
{
    // parse the function name
    if (current().isNot(TokenKind::KW_FUNC))
    {
        return errorExpected("'func'");
    }
    Location location = consume().getLoc();
    if (current().isNot(TokenKind::IDENTIFIER))
    {
        return errorExpected("identifier");
    }
    llvm::StringRef name = consume().getText();
    // parse the parameter list
    if (current().isNot(TokenKind::LPAREN))
    {
        return errorExpected("'('");
    }
    consume();
    std::vector<ParamNode*> params;
    if (current().isNot(TokenKind::RPAREN))
    {
        while (true)
        {
            ParamNode* param = parseParam();
            if (param->getType() != vslContext.getVoidType())
            {
                params.push_back(param);
            }
            else
            {
                diag.print<Diag::INVALID_PARAM_TYPE>(*param);
                // FIXME: delete the invalid ParamNode
            }
            if (current().isNot(TokenKind::COMMA))
            {
                break;
            }
            consume();
        }
    }
    if (current().isNot(TokenKind::RPAREN))
    {
        return errorExpected("')'");
    }
    consume();
    // parse the return type
    if (current().isNot(TokenKind::ARROW))
    {
        return errorExpected("'->'");
    }
    consume();
    const Type* returnType = parseType();
    // build the FunctionType
    std::vector<const Type*> paramTypes;
    paramTypes.resize(params.size());
    std::transform(params.begin(), params.end(), paramTypes.begin(),
        [](ParamNode* p)
        {
            return p->getType();
        });
    const FunctionType* ft = vslContext.getFunctionType(std::move(paramTypes),
        returnType);
    // parse an external function
    if (current().is(TokenKind::KW_EXTERNAL))
    {
        consume();
        if (current().isNot(TokenKind::LPAREN))
        {
            return errorExpected("'('");
        }
        consume();
        if (current().isNot(TokenKind::IDENTIFIER))
        {
            return errorExpected("identifier");
        }
        llvm::StringRef alias = consume().getText();
        if (current().isNot(TokenKind::RPAREN))
        {
            return errorExpected("')'");
        }
        consume();
        if (current().isNot(TokenKind::SEMICOLON))
        {
            return errorExpected("';'");
        }
        consume();
        return makeNode<ExtFuncNode>(location, name, std::move(params),
            returnType, ft, alias);
    }
    // parse a normal function
    Node* body = parseBlock();
    return makeNode<FunctionNode>(location, name, std::move(params), returnType,
        ft, body);
}

// param -> identifier ':' type
ParamNode* VSLParser::parseParam()
{
    Location location = current().getLoc();
    llvm::StringRef name;
    if (current().isNot(TokenKind::IDENTIFIER))
    {
        errorExpected("identifier");
        name = "";
        consume();
    }
    else
    {
        name = consume().getText();
    }
    if (consume().isNot(TokenKind::COLON))
    {
        // consumed here because the missing colon was likely a typo
        errorExpected("':'");
    }
    const Type* type = parseType();
    return makeNode<ParamNode>(location, name, type);
}

// return -> 'return' expr ';'
Node* VSLParser::parseReturn()
{
    if (current().isNot(TokenKind::KW_RETURN))
    {
        return errorExpected("'return'");
    }
    Location location = consume().getLoc();
    ExprNode* value;
    if (current().is(TokenKind::SEMICOLON))
    {
        // return without a value (void)
        consume();
        value = nullptr;
    }
    else
    {
        // return with a value
        value = parseExpr();
        if (current().isNot(TokenKind::SEMICOLON))
        {
            return errorExpected("';'");
        }
        consume();
    }
    return makeNode<ReturnNode>(location, value);
}

ExprNode* VSLParser::parseExprStmt()
{
    ExprNode* expr = parseExpr();
    if (current().isNot(TokenKind::SEMICOLON))
    {
        errorExpected("';'");
    }
    else
    {
        consume();
    }
    return expr;
}

// top down operator precedence (tdop) is used for expressions
ExprNode* VSLParser::parseExpr(int minPrec)
{
    ExprNode* lhs = parseUnaryOp();
    while (minPrec < getPrec(current().getKind()))
    {
        lhs = parseBinaryOp(lhs);
    }
    return lhs;
}

// unaryop -> ident | number | true | false | minus expr(call and up)
//          | lparen expr rparen
ExprNode* VSLParser::parseUnaryOp()
{
    Token t = consume();
    switch (t.getKind())
    {
    case TokenKind::IDENTIFIER:
        return makeNode<IdentNode>(t.getLoc(), t.getText());
    case TokenKind::NUMBER:
        return parseNumber(t);
    case TokenKind::KW_TRUE:
        return makeNode<LiteralNode>(t.getLoc(), llvm::APInt{ 1, 1 });
    case TokenKind::KW_FALSE:
        return makeNode<LiteralNode>(t.getLoc(), llvm::APInt{ 1, 0 });
    case TokenKind::MINUS:
        // only expression that can be parsed before unary minus is a func call
        return makeNode<UnaryNode>(t.getLoc(), t.getKind(),
            parseExpr(getPrec(TokenKind::LPAREN) - 1));
    case TokenKind::LPAREN:
        {
            ExprNode* expr = parseExpr();
            if (current().isNot(TokenKind::RPAREN))
            {
                errorExpected("')'");
            }
            consume();
            return expr;
        }
    default:
        errorExpected("unary operator, identifier, or number");
        return makeNode<LiteralNode>(t.getLoc(), llvm::APInt{ 32, 0 });
    }
}

// binaryop -> lhs (plus | minus | star | slash | percent | equal | not_equal
//                 | greater | greater_equal | less | less_equal) expr
//           | ternary | call | lparen expr rparen
ExprNode* VSLParser::parseBinaryOp(ExprNode* lhs)
{
    const Token& t = current();
    switch (t.getKind())
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
    case TokenKind::AND:
    case TokenKind::OR:
        // left associative
        consume();
        return makeNode<BinaryNode>(t.getLoc(), t.getKind(), lhs,
            parseExpr(getPrec(t.getKind())));
    case TokenKind::ASSIGN:
        // right associative
        consume();
        return makeNode<BinaryNode>(t.getLoc(), t.getKind(), lhs,
            parseExpr(getPrec(t.getKind()) - 1));
    case TokenKind::QUESTION:
        return parseTernary(lhs);
    case TokenKind::LPAREN:
        return parseCall(lhs);
    default:
        errorExpected("binary operator");
        return lhs;
    }
}

int VSLParser::getPrec(TokenKind k)
{
    switch (k)
    {
    case TokenKind::LPAREN:
        return 8;
    case TokenKind::STAR:
    case TokenKind::SLASH:
    case TokenKind::PERCENT:
        return 7;
    case TokenKind::PLUS:
    case TokenKind::MINUS:
        return 6;
    case TokenKind::GREATER:
    case TokenKind::GREATER_EQUAL:
    case TokenKind::LESS:
    case TokenKind::LESS_EQUAL:
        return 5;
    case TokenKind::EQUAL:
    case TokenKind::NOT_EQUAL:
        return 4;
    case TokenKind::AND:
    case TokenKind::OR:
        return 3;
    case TokenKind::QUESTION:
        return 2;
    case TokenKind::ASSIGN:
        return 1;
    default:
        return 0;
    }
}

// tenary -> condition question thenCase colon elseCase
// condition is passed as an argument from parseLed()
TernaryNode* VSLParser::parseTernary(ExprNode* condition)
{
    if (current().isNot(TokenKind::QUESTION))
    {
        errorExpected("'?'");
    }
    Token question = consume();
    Location location = question.getLoc();
    ExprNode* thenCase = parseExpr(getPrec(TokenKind::QUESTION) - 1);
    if (current().isNot(TokenKind::COLON))
    {
        errorExpected("':'");
    }
    consume();
    ExprNode* elseCase = parseExpr(getPrec(TokenKind::QUESTION) - 1);
    return makeNode<TernaryNode>(location, condition, thenCase, elseCase);
}

// call -> callee lparen arg (comma arg)* rparen
// callee is passed as an argument from parseLed()
CallNode* VSLParser::parseCall(ExprNode* callee)
{
    if (current().isNot(TokenKind::LPAREN))
    {
        errorExpected("'('");
    }
    Location location = consume().getLoc();
    std::vector<ArgNode*> args;
    if (current().isNot(TokenKind::RPAREN))
    {
        while (true)
        {
            args.emplace_back(parseCallArg());
            if (current().isNot(TokenKind::COMMA))
            {
                break;
            }
            consume();
        }
    }
    if (current().isNot(TokenKind::RPAREN))
    {
        errorExpected("')'");
    }
    consume();
    return makeNode<CallNode>(location, callee, args);
}

// arg -> identifier ':' expr
ArgNode* VSLParser::parseCallArg()
{
    llvm::StringRef name;
    if (current().isNot(TokenKind::IDENTIFIER))
    {
        errorExpected("identifier");
        name = "";
    }
    else
    {
        name = current().getText();
    }
    Location location = consume().getLoc();
    if (consume().isNot(TokenKind::COLON))
    {
        // consumed here because a missing colon is most likely a typo
        errorExpected("':'");
    }
    ExprNode* value = parseExpr();
    return makeNode<ArgNode>(location, name, value);
}

LiteralNode* VSLParser::parseNumber(const Token& token)
{
    const Location& location = token.getLoc();
    // make sure that the text inside is a valid 32bit integer
    llvm::APInt value;
    if (token.getText().getAsInteger(10, value))
    {
        diag.print<Diag::INVALID_INT>(token);
        value = 0;
    }
    else if (value.getActiveBits() > 32)
    {
        diag.print<Diag::OVERFLOW_DETECTED>(token);
    }
    value = value.zextOrTrunc(32);
    return makeNode<LiteralNode>(location, std::move(value));
}

// type -> 'Bool' | 'Int' | 'Void'
const Type* VSLParser::parseType()
{
    switch (consume().getKind())
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
