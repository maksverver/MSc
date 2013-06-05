// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "PredecessorLiftingStrategy.h"
#include "assert.h"

PredecessorLiftingStrategy::PredecessorLiftingStrategy(
    const ParityGame &game, const SmallProgressMeasures &spm,
    bool backward, bool stack )
    : LiftingStrategy(game), spm_(spm), backward_(backward), stack_(stack)
{
    assert(graph_.edge_dir() & StaticGraph::EDGE_PREDECESSOR);

    // Initialize data
    const verti V = game.graph().V();
    queued_ = new bool[V]();
    queue_ = new verti[V];
    queue_capacity_ = V;
    queue_begin_ = queue_end_ = queue_size_ = 0;
    for (verti i = 0; i < V; ++i)
    {
        queue_vertex(backward ? V - 1 - i : i);
    }
}

PredecessorLiftingStrategy::~PredecessorLiftingStrategy()
{
    delete[] queued_;
    delete[] queue_;
}

void PredecessorLiftingStrategy::queue_vertex(verti v)
{
    if (!queued_[v] && !spm_.is_top(v))
    {
        queued_[v] = true;
        queue_[queue_end_++] = v;
        if (queue_end_ == queue_capacity_) queue_end_ = 0;
        ++queue_size_;
        assert(queue_size_ <= queue_capacity_);
    }
}

void PredecessorLiftingStrategy::lifted(verti v)
{
    for ( StaticGraph::const_iterator it = graph_.pred_begin(v);
          it != graph_.pred_end(v); ++it )
    {
        queue_vertex(*it);
    }
}

verti PredecessorLiftingStrategy::next()
{
    if (queue_size_ == 0) return NO_VERTEX;

    // Remove an element from the queue
    verti res;
    if (stack_)
    {
        // Remove from the back of the queue
        if (queue_end_ == 0) queue_end_ = queue_capacity_;
        res = queue_[--queue_end_];
    }
    else
    {
        // Remove from the front of the queue
        res = queue_[queue_begin_++];
        if (queue_begin_ == queue_capacity_) queue_begin_ = 0;
    }
    --queue_size_;
    queued_[res] = false;
    return res;
}

PredecessorLiftingStrategy2::PredecessorLiftingStrategy2(
    const ParityGame &game, const SmallProgressMeasures &spm,
    bool backward, bool stack )
    : LiftingStrategy2(game), spm_(spm), backward_(backward), stack_(stack)
{
    assert(graph_.edge_dir() & StaticGraph::EDGE_PREDECESSOR);

    // Initialize data
    const verti V = game.graph().V();
    queue_ = new verti[V];
    queue_capacity_ = V;
    queue_begin_ = queue_end_ = queue_size_ = 0;
}

PredecessorLiftingStrategy2::~PredecessorLiftingStrategy2()
{
    delete[] queue_;
}

void PredecessorLiftingStrategy2::push(verti v)
{
    Logger::debug("push(%d)", v);
    queue_[queue_end_++] = v;
    if (queue_end_ == queue_capacity_) queue_end_ = 0;
    ++queue_size_;
    assert(queue_size_ <= queue_capacity_);
}

verti PredecessorLiftingStrategy2::pop()
{
    if (queue_size_ == 0) return NO_VERTEX;

    // Remove an element from the queue
    verti res;
    if (stack_)
    {
        // Remove from the back of the queue
        if (queue_end_ == 0) queue_end_ = queue_capacity_;
        res = queue_[--queue_end_];
    }
    else
    {
        // Remove from the front of the queue
        res = queue_[queue_begin_++];
        if (queue_begin_ == queue_capacity_) queue_begin_ = 0;
    }
    --queue_size_;
    Logger::debug("pop() -> %d", res);
    return res;
}

LiftingStrategy *PredecessorLiftingStrategyFactory::create(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    return new PredecessorLiftingStrategy(game, spm, backward_, stack_);
}

LiftingStrategy2 *PredecessorLiftingStrategyFactory::create2(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    return new PredecessorLiftingStrategy2(game, spm, backward_, stack_);
}
