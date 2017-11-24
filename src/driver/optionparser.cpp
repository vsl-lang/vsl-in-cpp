#include "driver/optionparser.hpp"
#include "llvm/Support/raw_ostream.h"
#include <cstring>

OptionParser::OptionParser()
    : action{ COMPILE }, optimize{ false }, infile { nullptr },
    outfile{ "a.out" }
{
}

void OptionParser::parse(int argc, const char* const* argv)
{
    for (int i = 1; i < argc; ++i)
    {
        const char* arg = argv[i];
        if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
        {
            action = DISPLAY_HELP;
        }
        else if (!strcmp(arg, "-l"))
        {
            action = REPL_LEX;
        }
        else if (!strcmp(arg, "-p"))
        {
            action = REPL_PARSE;
        }
        else if (!strcmp(arg, "-g"))
        {
            action = REPL_GENERATE;
        }
        else if (!strcmp(arg, "-o"))
        {
            if (i + 1 < argc)
            {
                outfile = argv[++i];
            }
            else
            {
                llvm::errs() << "Error: no output file given\n";
            }
        }
        else if (!strncmp(arg, "-O", 2))
        {
            if (arg[2] == '\0')
            {
                llvm::errs() << "Error: No optimization level specified.\n";
            }
            else if (arg[2] == '0')
            {
                optimize = false;
            }
            else if (arg[2] == '1')
            {
                optimize = true;
            }
            else
            {
                llvm::errs() << "Error: Unknown optimization level '" <<
                    &arg[2] << "'\n";
            }
        }
        // any other unknown flag, except "-" which means stdin
        else if (arg[0] == '-' && arg[1] != '\0')
        {
            llvm::errs() << "Error: unknown flag '" << arg << "'\n";
        }
        else if (infile != nullptr)
        {
            llvm::errs() <<
                "Error: VSL currently doesn't support multiple input files\n";
        }
        else
        {
            infile = arg;
        }
    }
}
