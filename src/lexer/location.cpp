#include "location.hpp"
#include <cstddef>
#include <ostream>

std::ostream& operator<<(std::ostream& os, const Location& location)
{
    return os << "file:" << location.line << ':' << location.col;
}

Location::Location(const char* pos, size_t line, size_t col)
    : pos{ pos }, line{ line }, col{ col }
{
}
