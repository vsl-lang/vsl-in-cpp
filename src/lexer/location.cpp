#include "lexer/location.hpp"

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const Location& location)
{
    return os << "file:" << location.line << ':' << location.col;
}

Location::Location(size_t line, size_t col)
    : line{ line }, col{ col }
{
}
