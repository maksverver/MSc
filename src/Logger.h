// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#ifdef USE_TIMER
#include "Timer.h"
#endif
#include <stdarg.h>
#include <stdlib.h>

#if __GNUC__ >= 3
#define PRINTF(decl) decl __attribute__((format(printf, 1, 2)));
#define NORETURN(decl) decl __attribute__((noreturn));
#else
#define PRINTF(decl)
#define NORETURN(decl)
#endif

/*! Logger base class.

    All members of this class are static.  They may be called directly,
    (e.g. as Logger::warn("foo")) or classes may inherit from this class so
    that the static methods can be called without prefixing them with the
    name of the class.
*/
class Logger
{
public:
    //! Severity of reported messages.
    enum Severity {
        LOG_DEBUG = 0, LOG_INFO = 1, LOG_WARN = 2, LOG_MESSAGE = 3,
        LOG_ERROR = 4, LOG_FATAL = 5, LOG_NONE = 6
    };

    //! Return the minimum severity of displayed messages
    static Severity severity() { return severity_; }

    //! Set the minimum severity of displayed messages
    static void severity(Severity s) { severity_ = s; }

    /*! Return whether messages with the given severity are displayed.

        Use this to determine whether messages of a certain severity level are
        displayed before performing expensive computations to generate these
        messages.  This allows code to execute quickly when the logging
        threshold is high.
    */
    static bool enabled(Severity s) { return s >= severity_; }

    inline static void PRINTF(debug(const char *fmt, ...));
    inline static void PRINTF(info(const char *fmt, ...));
    inline static void PRINTF(warn(const char *fmt, ...));
    inline static void PRINTF(message(const char *fmt, ...));
    inline static void PRINTF(error(const char *fmt, ...));
    inline static void PRINTF(NORETURN(fatal(const char *fmt, ...)));

private:
    /*! Prints a formatted message with the given severity to stderr,
        regardless of the current minimum severity level. */
    static void print_message(Severity severity, const char *fmt, va_list ap);

private:
#ifdef USE_TIMER
    static Timer timer_;        //! global timer shown in messages
#endif
    static Severity severity_;  //! minimum severity for displayed messages
};

#undef PRINTF
#undef NORETURN

void Logger::debug(const char *fmt, ...)
{
    /* Note: this is conditionally compiled in only if DEBUG is defined, so
       that semi-frequent calls to Logger::debug() don't slow down non-debug
       builds too much.  (For this to work, make sure arguments passed are
       simple enough that the compiler can optimize them away entirely!) */
#ifdef DEBUG
    if (enabled(LOG_DEBUG))
    {
        va_list ap;
        va_start(ap, fmt);
        print_message(LOG_DEBUG, fmt, ap);
        va_end(ap);
    }
#else  /* ndef DEBUG */
    (void)fmt;  // unused
#endif
}

void Logger::info(const char *fmt, ...)
{
    if (enabled(LOG_INFO))
    {
        va_list ap;
        va_start(ap, fmt);
        print_message(LOG_INFO, fmt, ap);
        va_end(ap);
    }
}

void Logger::warn(const char *fmt, ...)
{
    if (enabled(LOG_WARN))
    {
        va_list ap;
        va_start(ap, fmt);
        print_message(LOG_WARN, fmt, ap);
        va_end(ap);
    }
}

void Logger::message(const char *fmt, ...)
{
    if (enabled(LOG_MESSAGE))
    {
        va_list ap;
        va_start(ap, fmt);
        print_message(LOG_MESSAGE, fmt, ap);
        va_end(ap);
    }
}

void Logger::error(const char *fmt, ...)
{
    if (enabled(LOG_ERROR))
    {
        va_list ap;
        va_start(ap, fmt);
        print_message(LOG_ERROR, fmt, ap);
        va_end(ap);
    }
}

void Logger::fatal(const char *fmt, ...)
{
    if (enabled(LOG_FATAL))
    {
        va_list ap;
        va_start(ap, fmt);
        print_message(LOG_FATAL, fmt, ap);
        va_end(ap);
    }
    abort();
}

#endif /* ndef LOGGER_H_INCLUDED */
