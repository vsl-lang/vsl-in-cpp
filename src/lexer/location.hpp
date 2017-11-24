#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "llvm/Support/raw_ostream.h"
#include <cstddef>

class Location;

/**
 * Allows a Location to be printed to an output stream.
 *
 * @param os The stream to print to.
 * @param location The location to print.
 *
 * @returns The given output stream.
 */
llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const Location& location);

/**
 * Represents the location of an object in the source code of a program. Used
 * in error messages and the like.
 */
class Location
{
public:
    /**
     * Creates a Location.
     */
    Location() = default;
    /**
     * Creates a Location.
     *
     * @param line The line number.
     * @param col The column number.
     */
    Location(size_t line, size_t col);
    /** The line number. */
    size_t line;
    /** The column number. */
    size_t col;
};

#endif // LOCATION_HPP
