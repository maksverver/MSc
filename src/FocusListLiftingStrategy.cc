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
    bool backward, bool alternate, verti max_size, verti max_lifts )
    : LiftingStrategy(game), max_lifts_(max_lifts),
      pass_(1), lls_(game, backward, alternate),
      last_vertex_(NO_VERTEX), last_lifted_(false), num_lift_attempts_(0)
{
    focus_list_.reserve(max_size);
}

verti FocusListLiftingStrategy::next(verti prev_vertex, bool prev_lifted)
{
    verti res;
    switch (pass_)
    {
    case 1:
        res = pass1(prev_vertex, prev_lifted);
        break;

    case 2:
        res = pass2(prev_vertex, prev_lifted);
        break;

    default:
        res = NO_VERTEX;
        assert(0);
        break;
    }
    return res;
}

verti FocusListLiftingStrategy::pass1(verti prev_vertex, bool prev_lifted)
{
    last_vertex_ = prev_vertex;
    last_lifted_ = prev_lifted;

    /* Check if last vertex was successfully lifted */
    if (prev_lifted)
    {
        // Put successfully lifted vertex on the focus list
        assert(prev_vertex != NO_VERTEX);
        // FIXME: is this random factor necessary? If so, document this in
        // thesis.
        if (rand()%2)
        {
            focus_list_.push_back(std::make_pair(prev_vertex, initial_credit));
        }
    }

    if ( focus_list_.size() == focus_list_.capacity() ||
         (!focus_list_.empty() && num_lift_attempts_ >= game_.graph().V()) )
    {
        // Switch to pass 2
        info("Switching to focus list of size %d.", (int)focus_list_.size());
        pass_ = 2;
        num_lift_attempts_ = 0;
        return pass2(NO_VERTEX, false);
    }

    num_lift_attempts_ += 1;
    return lls_.next(last_vertex_, last_lifted_);
}

verti FocusListLiftingStrategy::pass2(verti prev_vertex, bool prev_lifted)
{
    if (prev_vertex == NO_VERTEX)
    {
        // Position at start of the focus list
        assert(!focus_list_.empty());
        read_pos_ = write_pos_ = focus_list_.begin();
    }
    else
    {
        // Adjust previous vertex credit and move to next position
        focus_list::value_type prev = *read_pos_++;
        assert(prev.first == prev_vertex);
        if (prev_lifted)
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
    }

    // Check if we've reached the end of the focus list
    if (read_pos_ == focus_list_.end())
    {
        focus_list_.erase(write_pos_, focus_list_.end());
        if (focus_list_.empty())
        {
            // Back to phase 1:
            assert(focus_list_.empty());
            info("Focus list exhausted.");
            pass_ = 1;
            num_lift_attempts_ = 0;
            return pass1(last_vertex_, last_lifted_);
        }
        else
        {
            // Restart at the beginning:
            read_pos_ = write_pos_ = focus_list_.begin();
        }
    }

    num_lift_attempts_ += 1;
    if (num_lift_attempts_ >= max_lifts_)
    {
        // Clear focus list and move back to phase 1:
        focus_list_.clear();
        info("Maximum lift attempts (%d) on focus list reached.", max_lifts_);
        pass_ = 1;
        num_lift_attempts_ = 0;
        return pass1(last_vertex_, last_lifted_);
    }

    // Return current item on the focus list
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
