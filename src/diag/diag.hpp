#ifndef DIAG_HPP
#define DIAG_HPP

#include "ast/node.hpp"
#include "ast/type.hpp"
#include "lexer/location.hpp"
#include "lexer/token.hpp"
#include "llvm/Support/raw_ostream.h"

/**
 * Prints error diagnostics.
 */
class Diag
{
public:
    /**
     * The kinds of diagnostic messages that can be printed. Refer to
     * `diags/diags.def` for more info.
     */
    enum Kind
    {
#define DIAG(kind, params, values) kind,
#include "diag/diags.def"
#undef DIAG
    };
    /**
     * Creates a Diag object.
     *
     * @param os Stream to print to.
     */
    Diag(llvm::raw_ostream& os);
    /**
     * Prints an error diagnostic. The parameter pack `args` changes based on
     * what you pass into `k`.
     *
     * @tparam k The kind of diagnostic to print.
     * @param args Arguments to include in the diagnostic.
     */
    template<Kind k, typename... Args>
    void print(Args&&... args);
    size_t getNumErrors() const;
    size_t getNumWarnings() const;

private:
    /** Stream to print to. */
    llvm::raw_ostream& os;
    /** Amount of errors that have been printed. */
    size_t errors;
    /** Amount of warnings that have been printed. */
    size_t warnings;
};

/**
 * Stuff that you shouldn't touch.
 */
namespace detail
{

/**
 * Prints only one argument.
 *
 * @param os Stream to print to.
 * @param arg Something to print.
 */
template<typename Arg>
void print(llvm::raw_ostream& os, Arg&& arg)
{
    os << std::forward<Arg>(arg);
}

/**
 * Prints an arbitrary amount of arguments.
 *
 * @param os Stream to print to.
 * @param arg1 First argument to print.
 * @param args All other arguments.
 */
template<typename Arg1, typename... Args>
void print(llvm::raw_ostream& os, Arg1&& arg1, Args&&... args)
{
    os << std::forward<Arg1>(arg1);
    print(os, std::forward<Args>(args)...);
}

/**
 * The different levels of diagnostics.
 */
enum class DiagLevel
{
    INTERNAL,
    FATAL,
    ERROR,
    WARNING
};

/**
 * Prints a diagnostic level.
 *
 * @param os Stream to print to.
 * @param level How severe the diagnostic is.
 */
void print(llvm::raw_ostream& os, DiagLevel level);

/**
 * Prints a diagnostic.
 *
 * @param os Stream to print to.
 * @param level How severe the diagnostic is.
 * @param args Arguments to include in the diagnostic.
 */
template<typename... Args>
void diagnose(llvm::raw_ostream& os, DiagLevel level, Args&&... args)
{
    print(os, level);
    print(os, std::forward<Args>(args)...);
}

/**
 * Prints a diagnostic with location info.
 *
 * @param os Stream to print to.
 * @param level How severe the diagnostic is.
 * @param l Location info for where the diagnostic occurred.
 * @param args Arguments to include in the diagnostic.
 */
template<typename... Args>
void diagnose(llvm::raw_ostream& os, DiagLevel level, Location l,
    Args&&... args)
{
    os.changeColor(llvm::raw_ostream::SAVEDCOLOR, true) << l << ": ";
    diagnose(os, level, std::forward<Args>(args)...);
}

/**
 * Prints one error diagnostic.
 *
 * @tparam k The kind of diagnostic to print.
 */
template<Diag::Kind k>
struct DiagPrinter
{
};

// for empty params, just put `(int=0)` to avoid a trailing comma error and the
//  compiler should optimize it out
// values must have a DiagLevel and (optionally) a Location at the beginning
// undef'd by including diags/diags.def
#define DIAG(kind, params, values) \
template<> \
struct DiagPrinter<Diag::kind> \
{ \
    static void print(llvm::raw_ostream& os, size_t& errors, size_t& warnings, \
        EXPAND params) \
    { \
        diagnose(os, DiagLevel::EXPAND values); \
        PROCESS values; \
    } \
};
#define EXPAND(...) __VA_ARGS__
#define PROCESS(level, ...) PROCESS_ ## level
#define PROCESS_INTERNAL PROCESS_FATAL
#define PROCESS_FATAL PROCESS_ERROR
#define PROCESS_ERROR ++errors
#define PROCESS_WARNING ++warnings
#include "diag/diags.def"
#undef PROCESS_WARNING
#undef PROCESS_ERROR
#undef PROCESS_FATAL
#undef PROCESS_INTERNAL
#undef PROCESS
#undef EXPAND
} // end namespace detail

template<Diag::Kind k, typename... Args>
void Diag::print(Args&&... args)
{
    detail::DiagPrinter<k>::print(os, errors, warnings,
        std::forward<Args>(args)...);
    os << '\n';
}

#endif // DIAG_HPP
