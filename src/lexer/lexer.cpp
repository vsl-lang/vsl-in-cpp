#include "lexer/lexer.hpp"

Lexer::Lexer(Diag& diag)
    : diag{ diag }
{
}

Lexer::~Lexer()
{
}

Diag& Lexer::getDiag() const
{
    return diag;
}
