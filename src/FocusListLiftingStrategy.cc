#include "FocusListLiftingStrategy.h"
#include <assert.h>

/*! Credit for a vertex when it is put on the focus list. */
static const unsigned initial_credit  = 2;

/*! Credit increase when a vertex on the focus list is succesfully lifted. */
static const unsigned credit_increase = 2;


FocusListLiftingStrategy::FocusListLiftingStrategy(
    const ParityGame &game, bool backward )
    : LiftingStrategy(game), backward_(backward), pass_(1),
      last_vertex_(NO_VERTEX), focus_list_(), focus_pos_(focus_list_.end()),
      focus_list_max_size_(0)
{
}

verti FocusListLiftingStrategy::next(verti prev_vertex, bool prev_lifted)
{
    verti res;
    switch (pass_)
    {
    case 1:
        res = pass1(prev_vertex,prev_lifted);
        break;

    case 2:
        res = pass2(prev_vertex,prev_lifted);
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
    /* Linear lifting */
    if (prev_vertex == NO_VERTEX)
    {
        // Select first vertex for pass 1
        assert(focus_list_.empty());
        return last_vertex_ = (backward_ ? graph_.V() - 1 : 0);
    }
    else
    {
        if (prev_lifted)
        {
            // Put succesfully lifted vertex on the focus list
            focus_list_.push_back(std::make_pair(prev_vertex, initial_credit));
        }

        if (last_vertex_ == (backward_ ? 0 : graph_.V() - 1))
        {
            // End of pass 1 reached.
            if (focus_list_.empty())
            {
                // No nodes were lifted; we're done.
                return NO_VERTEX;
            }
            else
            {
                // End of pass 1 -- focus list size is now at a local maximum
                focus_list_max_size_ = std::max( focus_list_max_size_,
                                                 focus_list_.size() );

                // Switch to pass 2
                pass_ = 2;
                return pass2(NO_VERTEX, false);
            }
        }
        else
        {
            // Return next vertex for pass 1
            return last_vertex_ = last_vertex_ + (backward_ ? -1 : +1);
        }
    }
}

verti FocusListLiftingStrategy::pass2(verti prev_vertex, bool prev_lifted)
{
    if (prev_vertex == NO_VERTEX)
    {
        // Position at start of the focus list
        assert(!focus_list_.empty());
        focus_pos_ = focus_list_.begin();
    }
    else
    {
        // Adjust previous vertex credit and move to next position
        focus_list::iterator old_pos = focus_pos_++;
        if (prev_lifted)
        {
            old_pos->second += credit_increase;
        }
        else
        {
            old_pos->second /= 2;
            if (old_pos->second == 0)
            {
                focus_list_.erase(old_pos);
            }
        }
    }

    // Check if we've reached the end of the focus list
    if (focus_pos_ == focus_list_.end())
    {
        if (focus_list_.empty())
        {
            // Focus list exhausted; move back to pass 1
            pass_ = 1;
            return pass1(NO_VERTEX, false);
        }
        else
        {
            // Restart at beginning of the list
            focus_pos_ = focus_list_.begin();
        }
    }

    // Return current item on the focus list
    return focus_pos_->first;
}

size_t FocusListLiftingStrategy::memory_use() const
{
    assert(focus_list_.size() <= focus_list_max_size_);
    return focus_list_max_size_*sizeof(focus_list::value_type);
}
