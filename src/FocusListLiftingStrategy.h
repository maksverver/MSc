#ifndef FOCUS_LIST_LIFTING_STRATEGY_H_INCLUDED
#define FOCUS_LIST_LIFTING_STRATEGY_H_INCLUDED

#include "SmallProgressMeasures.h"
#include <list>
#include <utility>

/*! This is an implementation of the "Focus List Approach" described in the
    Multi-Core Solver for Parity Games paper.

    It alternates between two passes. In the first pass, it iterates over the
    vertices linearly (forward or backward) and puts all vertices that are
    lifted on a focus list with a starting credit. In the second pass, nodes on
    the focus list are attempted to be lifted; if lifting succeeds, the credit
    for this node is increased linearly, and if lifting fails it is decreased
    exponentially. This pass is repeated until the focus list is empty.
*/

class FocusListLiftingStrategy : public LiftingStrategy
{
public:
    FocusListLiftingStrategy(const ParityGame &game, bool backward);
    verti next(verti prev_vertex, bool prev_lifted);
    size_t memory_use() const;

protected:
    verti pass1(verti prev_vertex, bool prev_lifted);
    verti pass2(verti prev_vertex, bool prev_lifted);

private:
    typedef std::list<std::pair<verti, unsigned> > focus_list;

    const bool backward_;               //!< indicates the direction to move
    int pass_;                          //!< current pass
    verti last_vertex_;                 //!< last vertex lifted linearly
    focus_list focus_list_;             //!< nodes on the focus list
    focus_list::iterator focus_pos_;    //!< current position in the focus list
    size_t focus_list_max_size_;  //!< peak number of entries in the focus list
};

#endif /* ndef FOCUS_LIST_LIFTING_STRATEGY_H_INCLUDED */
