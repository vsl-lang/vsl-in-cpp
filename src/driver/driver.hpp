#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "driver/optionparser.hpp"
#include "llvm/Support/raw_ostream.h"
#include <functional>

/**
 * Controls the entire compilation process.
 */
class Driver
{
public:
    /**
     * The driver's main point of execution.
     *
     * @param argc Argument count.
     * @param argv Argument vector.
     *
     * @returns 0 on success, 1 on failure.
     */
    int main(int argc, const char* const* argv);

private:
    /**
     * Displays help and usage information.
     *
     * @returns 0 on success, 1 on failure.
     */
    int displayHelp();
    /**
     * Does the standard compilation steps.
     */
    int compile();
    /**
     * Starts a REPL using the given evaluator function. The evaluator takes in
     * an input string reference as the first argument and an output stream
     * reference in the second argument.
     *
     * @param evaluator The function that evaluates the input.
     *
     * @returns 0 on success, 1 on failure.
     */
    int repl(
        std::function<void(const std::string&, llvm::raw_ostream&)> evaluator);
    /** The option parser. */
    OptionParser op;
};

#endif // DRIVER_HPP
