#include "lexer/lexer.hpp"

Lexer::Lexer(Diag& diag)
    : diag{ diag }
{
}

Lexer::~Lexer() = default;

Diag& Lexer::getDiag() const
{
    return diag;
}
