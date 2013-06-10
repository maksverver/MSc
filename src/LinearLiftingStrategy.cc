// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "LinearLiftingStrategy.h"

LinearLiftingStrategy::LinearLiftingStrategy(
    const ParityGame &game, bool alternate )
    : LiftingStrategy(), alternate_(alternate), dir_(0),
      last_vertex_(game.graph().V() - 1), vertex_(NO_VERTEX), failed_lifts_(0)
{
    max_failed_ = game.graph().V();
    if (alternate_) max_failed_ += max_failed_ - 1;
}

void LinearLiftingStrategy::lifted(verti v)
{
    (void)v;  // unused
    failed_lifts_ = 0;
}

verti LinearLiftingStrategy::next()
{
    if (failed_lifts_ >= max_failed_)
    {
        vertex_ = NO_VERTEX;
        return NO_VERTEX;
    }
    ++failed_lifts_;

    if (last_vertex_ == 0) return 0;

    if (vertex_ == NO_VERTEX)
    {
        dir_ = 0;
        vertex_ = 0;
    }
    else
    if (dir_ == 0)  // forward
    {
        if (vertex_ < last_vertex_)
        {
            ++vertex_;
        }
        else
        if (!alternate_)
        {
            vertex_ = 0;
        }
        else
        {
            dir_ = 1;
            --vertex_;
        }
    }
    else  // backward
    {
        if (vertex_ > 0)
        {
            --vertex_;
        }
        else
        if (!alternate_)
        {
            vertex_ = last_vertex_;
        }
        else
        {
            dir_ = 0;
            vertex_ = 1;
        }
    }
    return vertex_;
}

LiftingStrategy *LinearLiftingStrategyFactory::create(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    (void)spm;  // unused
    return new LinearLiftingStrategy(game, alternate_);
}
