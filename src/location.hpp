#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <cstddef>
#include <ostream>

class Location;

std::ostream& operator<<(std::ostream& os, const Location& location);

class Location
{
public:
    Location(const char* pos, size_t line, size_t col);
    const char* pos;
    size_t line;
    size_t col;
};

#endif // LOCATION_HPP
