// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "FocusListLiftingStrategy.h"
#include <assert.h>

/*! Credit for a vertex when it is put on the focus list. */
static const unsigned initial_credit  = 2;

/*! Credit increase when a vertex on the focus list is successfully lifted. */
static const unsigned credit_increase = 2;


FocusListLiftingStrategy::FocusListLiftingStrategy( const ParityGame &game,
    bool backward, bool alternate, verti max_size, long long max_lifts )
    : LiftingStrategy(game), max_lift_attempts_(max_lifts),
      phase_(1), num_lift_attempts_(0), lls_(game, backward, alternate)
{
    focus_list_.reserve(max_size);
}

void FocusListLiftingStrategy::lifted(verti vertex)
{
    if (phase_ == 1)
    {
        lls_.lifted(vertex);
        // FIXME: is this random factor necessary? If so, document in thesis.
        if (rand()%2)
        {
            focus_list_.push_back(std::make_pair(vertex, initial_credit));
            if (focus_list_.size() == focus_list_.capacity())
            {
                switch_to_phase(2);
            }
        }
    }
    else /* phase_ == 2 */
    {
        if (vertex == read_pos_->first)
        {
            prev_lifted_ = true;
        }
    }
}

void FocusListLiftingStrategy::switch_to_phase(int new_phase)
{
    num_lift_attempts_ = 0;
    if (new_phase == 2)
    {
        info("Switching to focus list of size %d.", (int)focus_list_.size());
        read_pos_ = write_pos_ = focus_list_.begin();
    }
    phase_ = new_phase;
}

verti FocusListLiftingStrategy::next()
{
    return phase_ == 1 ? phase1() : phase2();
}

verti FocusListLiftingStrategy::phase1()
{
    if (++num_lift_attempts_ >= graph_.V() && !focus_list_.empty())
    {
        switch_to_phase(2);
    }
    return lls_.next();
}

verti FocusListLiftingStrategy::phase2()
{
    if (num_lift_attempts_ > 0)
    {
        // Adjust previous vertex credit and move to next position
        focus_list::value_type prev = *read_pos_++;
        if (prev_lifted_)
        {
            prev.second += credit_increase;
            *write_pos_++ = prev;
        }
        else
        if (prev.second > 0)
        {
            prev.second /= 2;
            *write_pos_++ = prev;
        }
        // else, drop from list.
    }

    // Check if we've reached the end of the focus list
    if (read_pos_ == focus_list_.end())
    {
        focus_list_.erase(write_pos_, focus_list_.end());
        if (focus_list_.empty())
        {
            // Back to phase 1:
            info("Focus list exhausted.");
            switch_to_phase(1);
            return phase1();
        }
        else
        {
            // Restart at the beginning:
            read_pos_ = write_pos_ = focus_list_.begin();
        }
    }

    if (++num_lift_attempts_ >= max_lift_attempts_)
    {
        // Clear focus list and move back to phase 1:
        focus_list_.clear();
        info( "Maximum lift attempts (%lld) on focus list reached.",
              max_lift_attempts_ );
        switch_to_phase(1);
        return phase1();
    }

    // Return current item on the focus list
    prev_lifted_ = false;
    return read_pos_->first;
}

size_t FocusListLiftingStrategy::memory_use() const
{
    return sizeof(*this) + sizeof(focus_list_[0])*focus_list_.capacity();
}

LiftingStrategy *FocusListLiftingStrategyFactory::create(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    (void)spm;  // unused

    /* Ratio is absolute value if >1, or a fraction of the size of the game's
       vertex set if <= 1. */
    verti V = game.graph().V();
    verti max_size  = (size_ratio_ > 1) ? size_ratio_ : size_ratio_*V;
    if (max_size == 0) max_size = 1;
    if (max_size >  V) max_size = V;
    verti max_lifts = (verti)(lift_ratio_ * max_size);
    return new FocusListLiftingStrategy(
        game, backward_, alternate_, max_size, max_lifts );
}
