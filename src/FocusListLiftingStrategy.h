// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef FOCUS_LIST_LIFTING_STRATEGY_H_INCLUDED
#define FOCUS_LIST_LIFTING_STRATEGY_H_INCLUDED

#include "SmallProgressMeasures.h"
#include "LinearLiftingStrategy.h"
#include <utility>
#include <vector>

/*! This is an implementation of the "Focus List Approach" described in the
    Multi-Core Solver for Parity Games paper.

    It alternates between two passes. In the first pass, it iterates over the
    vertices linearly (forward or backward) and puts all vertices that are
    lifted on a focus list with a starting credit. In the second pass, nodes on
    the focus list are attempted to be lifted; if lifting succeeds, the credit
    for this node is increased linearly, and if lifting fails it is decreased
    exponentially. This pass is repeated until the focus list is empty.

    The focus list has a fixed maximum size. When this size is reached, the
    algorithm switches to pass 2.

    TODO: improve performance by using a fixed size list instead of a std::list
*/

class FocusListLiftingStrategy : public LiftingStrategy, public virtual Logger
{
public:
    FocusListLiftingStrategy( const ParityGame &game,
        bool backward, bool alternate, verti max_size, verti max_lifts );
    verti next(verti prev_vertex, bool prev_lifted);
    size_t memory_use() const;
    bool backward() const { return lls_.backward(); }
    bool max_size() const { return focus_list_.capacity(); }

protected:
    verti pass1(verti prev_vertex, bool prev_lifted);
    verti pass2(verti prev_vertex, bool prev_lifted);

private:
    typedef std::vector<std::pair<verti, unsigned> > focus_list;

    verti max_lifts_;                   //!< maximum lift attempts per list
    int pass_;                          //!< current pass

    // For pass 1:
    LinearLiftingStrategy lls_;         //!< strategy for pass 1
    verti last_vertex_;                 //!< last vertex selected in pass 1
    bool last_lifted_;                  //!< was last vertex lifted?
    verti num_lift_attempts_;           //!< number of consecutive lift attempts

    // For pass 2:
    focus_list focus_list_;             //!< nodes on the focus list
    focus_list::iterator read_pos_;     //!< current position in the focus list
    focus_list::iterator write_pos_;    //!< current position in the focus list
};


class FocusListLiftingStrategyFactory : public LiftingStrategyFactory
{
public:
    FocusListLiftingStrategyFactory(
        bool backward, bool alternate, double size_ratio, double lift_ratio )
        : backward_(backward), alternate_(alternate),
          size_ratio_(size_ratio > 0 ? size_ratio :  0.1),
          lift_ratio_(lift_ratio > 0 ? lift_ratio : 10.0) { };

    LiftingStrategy *create( const ParityGame &game,
                             const SmallProgressMeasures &spm );

private:
    const bool   backward_, alternate_;
    const double size_ratio_, lift_ratio_;
};

#endif /* ndef FOCUS_LIST_LIFTING_STRATEGY_H_INCLUDED */
