// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Don't include this file directly! Include SmallProgressMeasures.h instead.

inline int SmallProgressMeasures::vector_cmp(verti v, verti w, int N) const
{
    if (is_top(v)) return is_top(w) ? 0 : +1;   // v is top
    if (is_top(w)) return -1;                   // w is top, but v isn't

    for (int n = 0; n < N; ++n)
    {
        if (vec(v)[n] < vec(w)[n]) return -1;
        if (vec(v)[n] > vec(w)[n]) return +1;
    }

    return 0;
}

inline verti SmallProgressMeasures::get_ext_succ(verti v, bool take_max) const
{
    const verti *it  = game_.graph().succ_begin(v),
                *end = game_.graph().succ_end(v);

    assert(it != end);  /* assume we have at least one successor */

    int N = len(v);
    verti res = *it++;
    for ( ; it != end; ++it)
    {
        int d = vector_cmp(*it, res, N);
        if (take_max ? d > 0 : d < 0) res = *it;
    }
    return res;
}
