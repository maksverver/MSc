// Copyright (c) 2009-2013 University of Twente
// Copyright (c) 2009-2013 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2013 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2013 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <ctype.h>

#include "compatibility.h"

static int run_compatibility_checks();
static int dummy = run_compatibility_checks();

static int run_compatibility_checks()
{
    assert(sizeof(compat_int8_t)   == 1);
    assert(sizeof(compat_uint8_t)  == 1);
    assert(sizeof(compat_int16_t)  == 2);
    assert(sizeof(compat_uint16_t) == 2);
    assert(sizeof(compat_int32_t)  == 4);
    assert(sizeof(compat_uint32_t) == 4);
    assert(sizeof(compat_int64_t)  == 8);
    assert(sizeof(compat_uint64_t) == 8);
    (void)dummy;  /* squash unused variable warning */
    return 0;
}

int compat_strcasecmp(const char *s1, const char *s2)
{
    for (;;)
    {
        int d = tolower(*s1) - tolower(*s2);
        if (d || !*s1) return d;
        ++s1, ++s2;
    }
}

int compat_strncasecmp(const char *s1, const char *s2, size_t n)
{
    for (; n > 0; --n)
    {
        int d = tolower(*s1) - tolower(*s2);
        if (d || !*s1) return d;
        ++s1, ++s2;
    }
    return 0;
}
