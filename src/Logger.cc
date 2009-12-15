#include "Logger.h"
#include "timing.h"

#include <stdio.h>

Logger::Severity Logger::severity_ = FATAL;

/*! Prints an error message followed by a newline character, prefixed by the
    time in seconds spend by the program and displaying the type of error. */
void Logger::print_message(Severity severity, const char *fmt, va_list ap)
{
    fprintf(stderr, "[%7.3f] ", time_used());
    switch (severity)
    {
    case WARN:  fprintf(stderr, "WARNING: "); break;
    case ERROR: fprintf(stderr, "ERROR: "); break;
    case FATAL: fprintf(stderr, "FATAL ERROR: "); break;
    default:    break;
    }
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    fflush(stderr);
}
