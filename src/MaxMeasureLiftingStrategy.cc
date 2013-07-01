// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "MaxMeasureLiftingStrategy.h"
#include <algorithm>
#include <assert.h>

#include <stdio.h>  /* debug */

/* TODO: write short description of how this works! */

MaxMeasureLiftingStrategy2::MaxMeasureLiftingStrategy2(
    const ParityGame &game, const SmallProgressMeasures &spm,
    Order order, bool minimize )
        : LiftingStrategy2(), spm_(spm), order_(order), minimize_(minimize),
          next_id_(0),
          insert_id_(order < HEAP ? new compat_uint64_t[game.graph().V()] : 0),
          pq_pos_(new verti[game.graph().V()]),
          pq_(new verti[game.graph().V()]), pq_size_(0)
{
    std::fill(&pq_pos_[0], &pq_pos_[game.graph().V()], NO_VERTEX);
}

MaxMeasureLiftingStrategy2::~MaxMeasureLiftingStrategy2()
{
    delete[] insert_id_;
    delete[] pq_pos_;
    delete[] pq_;
}

void MaxMeasureLiftingStrategy2::move_up(verti i)
{
    // FIXME: this can be implemented with less swapping if I think harder.
    for (verti j; i > 0 && cmp(i, j = (i - 1)/2) > 0; i = j) swap(i, j);
}

void MaxMeasureLiftingStrategy2::move_down(verti i)
{
    // FIXME: this can be implemented with less swapping if I think harder.
    for (;;)
    {
        verti j = 2*i + 1;
        verti k = 2*i + 2;
        int d = j < pq_size_ ? cmp(i, j) : 1;
        int e = k < pq_size_ ? cmp(i, k) : 1;

        if (d < 0 && e < 0)
        {
            // both children are larger than current node
            if (cmp(j, k) >= 0)
            {
                // left child is largest
                swap(i, j);
                i = j;
            }
            else
            {
                // right child is largest;
                swap(i, k);
                i = k;
            }
        }
        else
        if (d < 0)
        {
            // left child is larger
            swap(i, j);
            i = j;
        }
        else
        if (e < 0)
        {
            // right child is larger
            swap(i, k);
            i = k;
        }
        else
        {
            // both children are smaller; we're done
            break;
        }
    }
}

void MaxMeasureLiftingStrategy2::swap(verti i, verti j)
{
    verti v = pq_[i], w = pq_[j];
    pq_[i] = w;
    pq_[j] = v;
    pq_pos_[w] = i;
    pq_pos_[v] = j;
}

void MaxMeasureLiftingStrategy2::push(verti v)
{
    Logger::debug("push(%d)", v);
    assert(pq_pos_[v] == NO_VERTEX);
    pq_[pq_size_] = v;
    pq_pos_[v] = pq_size_;
    ++pq_size_;
    if (insert_id_) insert_id_[v] = next_id_++;
    bumped_.push_back(pq_pos_[v]);
}

void MaxMeasureLiftingStrategy2::bump(verti v)
{
    Logger::debug("bump(%d)", v);
    bumped_.push_back(pq_pos_[v]);
}

verti MaxMeasureLiftingStrategy2::pop()
{
#ifdef DEBUG
    static long long ops;
    ops += bumped_.size() + 1;
#endif

    if (!bumped_.empty())
    {
        // Move bumped vertices up the heap.
        std::sort(bumped_.begin(), bumped_.end());
        for (size_t i = 0; i < bumped_.size(); ++i)
        {
            if (i == 0 || bumped_[i] > bumped_[i - 1]) move_up(bumped_[i]);
            move_up(bumped_[i]);
        }
        if (minimize_)
        {
            /* Note: minimization is a bit trickier than maximization, since
               we need to move bumped vertices down the heap (rather than up
               when maximizing) but pushed vertices still need to move up.

               Unfortunately, we can't easily distinguish between bumped or
               pushed or pushed-and-then-bumped vertices, so the easiest safe
               way to handle the situation is to move up first, and then down.

               FIXME: optimize this.
            */

            // Move bumped vertices down the heap.
            std::reverse(bumped_.rbegin(), bumped_.rend());
            for (size_t i = 0; i < bumped_.size(); ++i)
            {
                if (i == 0 || bumped_[i] < bumped_[i - 1]) move_down(bumped_[i]);
                move_down(bumped_[i]);
            }
        }
        bumped_.clear();
    }

    if (pq_size_ == 0) return NO_VERTEX;

#ifdef DEBUG
    if (ops >= pq_size_)
    {
        Logger::debug("checking heap integrity");
        assert(check());
        ops -= pq_size_;
    }
#endif

    // Extract top element from the heap.
    verti v = pq_[0];
    Logger::debug("pop() -> %d", v);
    pq_pos_[v] = NO_VERTEX;
    if (--pq_size_ > 0)
    {
        pq_[0] = pq_[pq_size_];
        pq_pos_[pq_[0]] = 0;
        move_down(0);
    }
    return v;
}

static int cmp_ids(compat_uint64_t x, compat_uint64_t y)
{
    return (x > y) - (x < y);
}

int MaxMeasureLiftingStrategy2::cmp(verti i, verti j)
{
    verti v = pq_[i], w = pq_[j];
    int d = spm_.vector_cmp( spm_.get_successor(v),
                             spm_.get_successor(w), spm_.len_ );
    if (d != 0) return minimize_ ? -d : +d;

    // Tie-break on insertion order: smallest insert-id first in queue
    // mode, or largest insert-id first in stack mode.
    switch (order_)
    {
    case STACK: return cmp_ids(insert_id_[v], insert_id_[w]);
    case QUEUE: return cmp_ids(insert_id_[w], insert_id_[v]);
    default:    return 0;
    }
}

bool MaxMeasureLiftingStrategy2::check()
{
    for (verti i = 1; i < pq_size_; ++i)
    {
        if (cmp(i, (i - 1)/2) > 0) return false;
    }

    for (verti i = 0; i < pq_size_; ++i)
    {
        if (pq_pos_[pq_[i]] != i) return false;
    }

    const verti V = spm_.game().graph().V();
    for (verti v = 0; v < V; ++v)
    {
        if (pq_pos_[v] != NO_VERTEX)
        {
            if (pq_[pq_pos_[v]] != v) return false;
        }
    }

    return true;
}

bool MaxMeasureLiftingStrategyFactory::supports_version(int version)
{
    return version == 2;
}

LiftingStrategy *MaxMeasureLiftingStrategyFactory::create(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    return 0;
}

LiftingStrategy2 *MaxMeasureLiftingStrategyFactory::create2(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    return new MaxMeasureLiftingStrategy2(game, spm, order_, minimize_);
}
