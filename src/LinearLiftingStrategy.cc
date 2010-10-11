// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "LinearLiftingStrategy.h"

LinearLiftingStrategy::LinearLiftingStrategy(
    const ParityGame &game, bool backward, bool alternate )
    : LiftingStrategy(game), backward_(backward), alternate_(alternate),
      dir_(0), failed_lifts_(0)
{
}

verti LinearLiftingStrategy::next(verti prev_vertex, bool prev_lifted)
{
    const verti last_vertex = graph_.V() - 1;
    if (last_vertex == 0) return 0;

    if (prev_vertex == NO_VERTEX)
    {
        /* First vertex; pick either first or last depending on direction. */
        dir_ = backward_;
        return dir_ ? last_vertex : 0;
    }

    if (prev_lifted)
    {
        failed_lifts_ = 0;
    }
    else
    {
        failed_lifts_ += 1;
        if (failed_lifts_ == graph_.V()) return NO_VERTEX;
    }

    if (dir_ == 0)  // forward
    {
        if (prev_vertex != last_vertex) return prev_vertex + 1;
        if (!alternate_) return 0;
        dir_ = 1;
        return last_vertex - 1;
    }
    else  // backward
    {
        if (prev_vertex != 0) return prev_vertex - 1;
        if (!alternate_) return last_vertex;
        dir_ = 0;
        return 1;
    }
}

LiftingStrategy *LinearLiftingStrategyFactory::create(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    (void)spm;  // unused
    return new LinearLiftingStrategy(game, backward_, alternate_);
}
