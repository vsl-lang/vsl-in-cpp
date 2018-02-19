#include "parser/vslparser.hpp"
#include "ast/opKind.hpp"

VSLParser::VSLParser(VSLContext& vslCtx, Lexer& lexer)
    : vslCtx{ vslCtx }, lexer{ lexer }, diag{ lexer.getDiag() }
{
}

// program -> decl* end
void VSLParser::parse()
{
    while (current().isNot(TokenKind::END))
    {
        if (DeclNode* decl = parseDecl())
        {
            // the parsed decl is valid
            vslCtx.setGlobal(decl);
        }
    }
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

void VSLParser::errorExpected(const char* s)
{
    diag.print<Diag::EXPECTED_BUT_FOUND>(s, current());
}

void VSLParser::errorUnexpected(const Token& token)
{
    diag.print<Diag::UNEXPECTED_TOKEN>(token);
}

// decl -> function | variable
DeclNode* VSLParser::parseDecl()
{
    AccessMod access;
    switch (current().getKind())
    {
    case TokenKind::KW_PUBLIC:
        access = AccessMod::PUBLIC;
        break;
    case TokenKind::KW_PRIVATE:
        access = AccessMod::PRIVATE;
        break;
    default:
        errorExpected("access modifier");
        access = AccessMod::PRIVATE; // assumed for now
    }
    consume();
    switch (current().getKind())
    {
    case TokenKind::KW_FUNC:
        return parseFunction(access);
    case TokenKind::KW_VAR:
    case TokenKind::KW_LET:
        return parseVariable(access);
    default:
        errorUnexpected(consume());
        return nullptr;
    }
}

// function -> accessmod funcInterface block | extfunc
// extfunc -> funcInterface external lparen ident rparen semicolon
// funcInterface -> func ident lparen param (comma param)* rparen arrow type
// accessmod provided by caller
FuncInterfaceNode* VSLParser::parseFunction(AccessMod access)
{
    // parse the function name
    if (current().isNot(TokenKind::KW_FUNC))
    {
        errorExpected("'func'");
        return nullptr;
    }
    Location location = consume().getLoc();
    if (current().isNot(TokenKind::IDENTIFIER))
    {
        errorExpected("identifier");
        return nullptr;
    }
    llvm::StringRef name = consume().getText();
    // parse the parameter list
    if (current().isNot(TokenKind::LPAREN))
    {
        errorExpected("'('");
        return nullptr;
    }
    consume();
    std::vector<ParamNode*> params;
    if (current().isNot(TokenKind::RPAREN))
    {
        while (true)
        {
            ParamNode* param = parseParam();
            if (param)
            {
                params.push_back(param);
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
        errorExpected("')'");
        return nullptr;
    }
    consume();
    // parse the return type
    if (current().isNot(TokenKind::ARROW))
    {
        errorExpected("'->'");
        return nullptr;
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
    const FunctionType* ft = vslCtx.getFunctionType(std::move(paramTypes),
        returnType);
    // parse an external function
    if (current().is(TokenKind::KW_EXTERNAL))
    {
        consume();
        if (current().isNot(TokenKind::LPAREN))
        {
            errorExpected("'('");
            return nullptr;
        }
        consume();
        if (current().isNot(TokenKind::IDENTIFIER))
        {
            errorExpected("identifier");
            return nullptr;
        }
        llvm::StringRef alias = consume().getText();
        if (current().isNot(TokenKind::RPAREN))
        {
            errorExpected("')'");
            return nullptr;
        }
        consume();
        if (current().isNot(TokenKind::SEMICOLON))
        {
            errorExpected("';'");
            return nullptr;
        }
        consume();
        return makeNode<ExtFuncNode>(location, access, name, std::move(params),
            returnType, ft, alias);
    }
    // parse a normal function
    BlockNode* body = parseBlock();
    if (!body)
    {
        return nullptr;
    }
    return makeNode<FunctionNode>(location, access, name, std::move(params),
        returnType, ft, *body);
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
    if (!type->isValid())
    {
        diag.print<Diag::INVALID_PARAM_TYPE>(location, *type);
        return nullptr;
    }
    return makeNode<ParamNode>(location, name, type);
}

// variable -> accessmod (var | let) identifier colon type assign expr semicolon
// accessmod provided by caller if applicable
VariableNode* VSLParser::parseVariable(AccessMod access)
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
        errorExpected("'let' or 'var'");
        return nullptr;
    }
    Location location = consume().getLoc();
    if (current().isNot(TokenKind::IDENTIFIER))
    {
        errorExpected("identifier");
        return nullptr;
    }
    llvm::StringRef name = consume().getText();
    if (current().isNot(TokenKind::COLON))
    {
        errorExpected("':'");
        return nullptr;
    }
    consume();
    const Type* type = parseType();
    if (!type)
    {
        return nullptr;
    }
    if (current().isNot(TokenKind::ASSIGN))
    {
        errorExpected("'='");
        return nullptr;
    }
    consume();
    ExprNode* init = parseExpr();
    if (current().isNot(TokenKind::SEMICOLON))
    {
        errorExpected("';'");
        return nullptr;
    }
    consume();
    return makeNode<VariableNode>(location, access, name, type, init,
        constness);
}

// statements -> statement*
std::vector<Node*> VSLParser::parseStatements()
{
    std::vector<Node*> statements;
    while (!empty() && current().isNot(TokenKind::RBRACE) &&
        current().isNot(TokenKind::END))
    {
        Node* statement = parseStatement();
        if (statement)
        {
            statements.push_back(statement);
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
    case TokenKind::SEMICOLON:
        return makeNode<EmptyNode>(consume().getLoc());
    case TokenKind::KW_FUNC:
        // function within a function
        diag.print<Diag::FUNCEPTION>(consume().getLoc());
        return nullptr;
    default:
        errorUnexpected(consume());
        return nullptr;
    }
}

// block -> lbrace statements rbrace
BlockNode* VSLParser::parseBlock()
{
    if (current().isNot(TokenKind::LBRACE))
    {
        errorExpected("'{'");
        return nullptr;
    }
    Location location = consume().getLoc();
    std::vector<Node*> statements = parseStatements();
    if (current().isNot(TokenKind::RBRACE))
    {
        errorExpected("'}'");
        return nullptr;
    }
    consume();
    return makeNode<BlockNode>(location, std::move(statements));
}

// conditional -> if lparen expr rparen statement (else statement)?
IfNode* VSLParser::parseIf()
{
    if (current().isNot(TokenKind::KW_IF))
    {
        errorExpected("'if'");
        return nullptr;
    }
    Location location = consume().getLoc();
    // parse condition wrapped in parens
    if (current().isNot(TokenKind::LPAREN))
    {
        errorExpected("'('");
        return nullptr;
    }
    consume();
    ExprNode* condition = parseExpr();
    if (!condition)
    {
        return nullptr;
    }
    if (current().isNot(TokenKind::RPAREN))
    {
        errorExpected("')'");
        return nullptr;
    }
    consume();
    // parse then case
    Node* thenCase = parseStatement();
    if (!thenCase)
    {
        return nullptr;
    }
    // parse else case
    Node* elseCase;
    if (current().is(TokenKind::KW_ELSE))
    {
        consume();
        elseCase = parseStatement();
    }
    else
    {
        elseCase = nullptr;
    }
    return makeNode<IfNode>(location, *condition, *thenCase, elseCase);
}

// return -> 'return' expr ';'
ReturnNode* VSLParser::parseReturn()
{
    if (current().isNot(TokenKind::KW_RETURN))
    {
        errorExpected("'return'");
        return nullptr;
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
            errorExpected("';'");
            return nullptr;
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
    while (lhs && minPrec < getPrec(current().getKind()))
    {
        lhs = parseBinaryOp(*lhs);
    }
    return lhs;
}

// unaryop -> ident | number | true | false | unaryexpr | lparen expr rparen
// unaryexpr -> (minus | not) expr(minPrec = call-1)
// only expression that can be parsed before unary is a function call
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
    case TokenKind::NOT:
        {
            ExprNode* expr = parseExpr(getPrec(TokenKind::LPAREN) - 1);
            if (!expr)
            {
                return nullptr;
            }
            return makeNode<UnaryNode>(t.getLoc(),
                tokenKindToUnary(t.getKind()), *expr);
        }
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
        errorExpected("expression");
        return nullptr;
    }
}

// binaryop -> binaryexpr | ternary | call
ExprNode* VSLParser::parseBinaryOp(ExprNode& lhs)
{
    const Token& t = current();
    switch (t.getKind())
    {
        // the ternary/call operators aren't actually binary operators, but they
        //  come after an expression so that's good enough
    case TokenKind::QUESTION:
        return parseTernary(lhs);
    case TokenKind::LPAREN:
        return parseCall(lhs);
    default:
        return parseBinaryExpr(lhs);
    }
}

// binaryexpr -> lhs (star | slash | percent | plus | minus | greater
//                   | greater_equal | less | less_equal | equal | not_equal
//                   | and | or | assign) expr
BinaryNode* VSLParser::parseBinaryExpr(ExprNode& lhs)
{
    Token t = consume();
    TokenKind k = t.getKind();
    BinaryKind op = tokenKindToBinary(k);
    int minPrec = getPrec(k);
    switch (op)
    {
    case BinaryKind::STAR:
    case BinaryKind::SLASH:
    case BinaryKind::PERCENT:
    case BinaryKind::PLUS:
    case BinaryKind::MINUS:
    case BinaryKind::GREATER:
    case BinaryKind::GREATER_EQUAL:
    case BinaryKind::LESS:
    case BinaryKind::LESS_EQUAL:
    case BinaryKind::EQUAL:
    case BinaryKind::NOT_EQUAL:
    case BinaryKind::AND:
    case BinaryKind::OR:
        // left associative
        break;
    case BinaryKind::ASSIGN:
        // right associative
        --minPrec;
        break;
    default:
        diag.print<Diag::NOT_A_BINARY_OP>(t);
        return nullptr;
    }
    ExprNode* rhs = parseExpr(minPrec);
    if (!rhs)
    {
        return nullptr;
    }
    return makeNode<BinaryNode>(t.getLoc(), op, lhs, *rhs);
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

// ternary -> condition question thenCase colon elseCase
// condition provided by caller
TernaryNode* VSLParser::parseTernary(ExprNode& condition)
{
    if (current().isNot(TokenKind::QUESTION))
    {
        errorExpected("'?'");
        return nullptr;
    }
    Token question = consume();
    Location location = question.getLoc();
    ExprNode* thenCase = parseExpr(getPrec(TokenKind::QUESTION) - 1);
    if (!thenCase)
    {
        return nullptr;
    }
    if (current().isNot(TokenKind::COLON))
    {
        errorExpected("':'");
        return nullptr;
    }
    consume();
    ExprNode* elseCase = parseExpr(getPrec(TokenKind::QUESTION) - 1);
    if (!elseCase)
    {
        return nullptr;
    }
    return makeNode<TernaryNode>(location, condition, *thenCase, *elseCase);
}

// call -> callee lparen arg (comma arg)* rparen
// callee provided by caller
CallNode* VSLParser::parseCall(ExprNode& callee)
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
            auto* arg = parseCallArg();
            if (arg)
            {
                args.emplace_back(arg);
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
        return nullptr;
    }
    name = current().getText();
    Location location = consume().getLoc();
    if (consume().isNot(TokenKind::COLON))
    {
        errorExpected("':'");
        return nullptr;
    }
    ExprNode* value = parseExpr();
    if (!value)
    {
        return nullptr;
    }
    return makeNode<ArgNode>(location, name, *value);
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
        return vslCtx.getBoolType();
    case TokenKind::KW_INT:
        return vslCtx.getIntType();
    case TokenKind::KW_VOID:
        return vslCtx.getVoidType();
    default:
        errorExpected("type");
        return vslCtx.getErrorType();
    }
}

template<typename NodeT, typename... Args>
typename std::enable_if<std::is_base_of<Node, NodeT>::value, NodeT*>::type
VSLParser::makeNode(Args&&... args) const
{
    auto node = std::make_unique<NodeT>(std::forward<Args>(args)...);
    NodeT* nodePtr = node.get();
    vslCtx.addNode(std::move(node));
    return nodePtr;
}
