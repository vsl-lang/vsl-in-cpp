#include "lexer/location.hpp"
#include <cstddef>
#include <ostream>

std::ostream& operator<<(std::ostream& os, const Location& location)
{
    return os << "file:" << location.line << ':' << location.col;
}

Location::Location(size_t line, size_t col)
    : line{ line }, col{ col }
{
}
