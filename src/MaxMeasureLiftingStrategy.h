// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MAX_MEASURE_LIFTING_STRATEGY_H_INCLUDED
#define MAX_MEASURE_LIFTING_STRATEGY_H_INCLUDED

#include "SmallProgressMeasures.h"
#include <vector>
#include <set>
#include <utility>

/*! \ingroup LiftingStrategies

    A lifting strategy that propagate maximum measures first.

    Conceptually this is a specialization of the predecessor lifting strategy.
    However, of all feasible vertices to select for the next lifting attempt,
    one is chosen specifically that has the largest maximum progress measure
    vector.

    The implementation uses a custom heap structure to act as a priority queue.
    Special care must be taken to maintain the heap property, because lifting
    vertices changes the associated progress measure!
*/
class MaxMeasureLiftingStrategy : public LiftingStrategy
{
public:
    enum Order { QUEUE = 0, STACK = 1, HEAP = 2 }; 

    MaxMeasureLiftingStrategy( const ParityGame &game,
                               const SmallProgressMeasures &spm,
                               bool backward, Order order );
    ~MaxMeasureLiftingStrategy();

    void lifted(verti v);
    verti next();

protected:

    /*! Moves the element at index i up the heap until the heap property
        is restored. */
    void move_up(verti i);

    /*! Moves the element at index i down the heap until the heap property
        is restored. */
    void move_down(verti i);

    /*! Swaps the elements at indices i and j in the heap. */
    void swap(verti i, verti j);

    /*! Pushes the vertex into the queue, or restores the heap property after
        vertex v has been modified. */
    void push(verti v);

    /*! Removes the vertex from the queue, if it is present. */
    //void remove(verti v);

    /*! Returns the top element in the heap. */
    verti top() { return pq_[0]; }

    /*! Pops the top element from the heap and restores the heap property */
    void pop();

    /*! Compares the vertices referred through indices i and j in the heap. */
    int cmp(verti i, verti j);

    /*! Checks if the queue satisfies the heap property (used for debugging) */
    bool check();

private:
    MaxMeasureLiftingStrategy(const MaxMeasureLiftingStrategy &);
    MaxMeasureLiftingStrategy &operator=(const MaxMeasureLiftingStrategy &);

private:
    const SmallProgressMeasures &spm_;  //!< SPM instance being solved
    const Order order_;                 //!< vertex extraction order

    compat_uint64_t next_id_;        //!< number of insertions
    compat_uint64_t * insert_id_;    //!< for each vertex: last insertion time

    verti * const pq_pos_;      //!< for each vertex: position in the p.q. or -1
    verti * const pq_;          //!< priority queue of lifted vertices
    verti pq_size_;             //!< priority queue size
};

/*! \ingroup LiftingStrategies
    A factory class for MaxMeasureLiftingStrategy instances. */
class MaxMeasureLiftingStrategyFactory : public LiftingStrategyFactory
{
public:
    MaxMeasureLiftingStrategyFactory( bool backward = false,
        MaxMeasureLiftingStrategy::Order order = MaxMeasureLiftingStrategy::HEAP )
        : backward_(backward), order_(order) { };

    //! Return a new MaxMeasureLiftingStrategy instance. 
    LiftingStrategy *create( const ParityGame &game,
                             const SmallProgressMeasures &spm );

private:
    const bool backward_;
    const MaxMeasureLiftingStrategy::Order order_;
};

#endif /* ndef PREDECESSOR_LIFTING_STRATEGY_H_INCLUDED */
