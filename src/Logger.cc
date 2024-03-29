// Copyright (c) 2009-2013 University of Twente
// Copyright (c) 2009-2013 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2013 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2013 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "Logger.h"

#include <stdio.h>

Logger::Severity Logger::severity_ = LOG_FATAL;
#ifdef USE_TIMER
Timer Logger::timer_;
#endif

/*! Prints an error message followed by a newline character, prefixed by the
    time in seconds spend by the program and displaying the type of error. */
void Logger::print_message(Severity severity, const char *fmt, va_list ap)
{
#ifdef USE_TIMER
    fprintf(stderr, "[%7.3f] ", timer_.elapsed());
#endif
#ifdef WITH_MPI
    extern int mpi_rank, mpi_size;
    if (mpi_size > 1) fprintf(stderr, "(%d/%d) ", mpi_rank, mpi_size);
#endif
    switch (severity)
    {
    case LOG_WARN:  fprintf(stderr, "WARNING: "); break;
    case LOG_ERROR: fprintf(stderr, "ERROR: "); break;
    case LOG_FATAL: fprintf(stderr, "FATAL ERROR: "); break;
    default:    break;
    }
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    fflush(stderr);
}
