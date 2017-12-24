#include "diag/diag.hpp"

Diag::Diag(llvm::raw_ostream& os)
    : os { os }, errors{ 0 }, warnings{ 0 }
{
}

size_t Diag::getNumErrors() const
{
    return errors;
}

size_t Diag::getNumWarnings() const
{
    return warnings;
}

void detail::print(llvm::raw_ostream& os, DiagLevel level)
{
    switch (level)
    {
    case DiagLevel::INTERNAL:
        os.changeColor(llvm::raw_ostream::RED, true) << "INTERNAL";
        break;
    case DiagLevel::FATAL:
        os.changeColor(llvm::raw_ostream::RED, true) << "fatal";
        break;
    case DiagLevel::ERROR:
        os.changeColor(llvm::raw_ostream::RED, true) << "error";
        break;
    case DiagLevel::WARNING:
        os.changeColor(llvm::raw_ostream::YELLOW, true) << "warning";
        break;
    }
    os << ": ";
    os.resetColor();
}
