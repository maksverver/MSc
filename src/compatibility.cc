#include <assert.h>
#include <ctype.h>

#include "compatibility.h"

static struct CompatibilityChecks {
    CompatibilityChecks() {
        assert(sizeof(int8_t)   == 1);
        assert(sizeof(uint8_t)  == 1);
        assert(sizeof(int16_t)  == 2);
        assert(sizeof(uint16_t) == 2);
        assert(sizeof(int32_t)  == 4);
        assert(sizeof(uint32_t) == 4);
        assert(sizeof(int64_t)  == 8);
        assert(sizeof(uint64_t) == 8);
    }
} checks;

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
