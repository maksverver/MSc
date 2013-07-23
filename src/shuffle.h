#ifndef SHUFFLE_H_INCLUDED

#include <algorithm>
#include <vector>
#include <stdlib.h>

/* Randomly shuffle the given vector.  This is similar to std::random_shuffle
   on a vector,  except that this uses rand() while the random source for
   std::random_shuffle is unspecified. */
template<class T>
static void shuffle_vector(std::vector<T> &v)
{
    size_t n = v.size();
    for (size_t i = 0; i < n; ++i)
    {
        std::swap(v[i], v[i + rand()%(n - i)]);
    }
}

#endif /* ndef SHUFFLE_H_INCLUDED */
