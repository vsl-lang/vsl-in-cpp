#ifndef OPTIONPARSER_HPP
#define OPTIONPARSER_HPP

/**
 * Parses command-line arguments.
 */
class OptionParser
{
public:
    /**
     * Indicates what action the compiler should take.
     */
    enum Action
    {
        /** Compile a source file into an object file. */
        COMPILE,
        /** Display help and usage information. */
        DISPLAY_HELP,
        /** Emits a list of tokens. */
        REPL_LEX,
        /** Emits an abstract syntax tree. */
        REPL_PARSE,
        /** Emits LLVM IR. */
        REPL_GENERATE
    };
    /**
     * Creates an OptionParser object.
     */
    OptionParser();
    /**
     * Parses all the arguments.
     *
     * @param argc Argument count.
     * @param argv Argument vector.
     */
    void parse(int argc, const char* const* argv);
    /** The action the compiler should take. */
    Action action;
    /** True if the compiler should optimize the LLVM IR output. */
    bool optimize;
    /** The file name to take input from. */
    const char* infile;
    /** The file name to emit output to. */
    const char* outfile;
};

#endif // OPTIONPARSER_HPP
