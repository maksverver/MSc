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

MaxMeasureLiftingStrategy::MaxMeasureLiftingStrategy(
    const ParityGame &game, const SmallProgressMeasures &spm,
    bool backward, Order order )
        : LiftingStrategy(game), spm_(spm), order_(order), next_id_(0),
          insert_id_(order < HEAP ? new compat_uint64_t[graph_.V()] : NULL),
          pq_pos_(new verti[graph_.V()]), pq_(new verti[graph_.V()])
{
    const verti V = graph_.V();

    // Initialize queue
    pq_size_ = 0;
    for (verti i = 0; i < V; ++i)
    {
        const verti v = backward ? V - 1 - i : i;
        if (!spm_.is_top(v))
        {
            pq_pos_[v] = (verti)-1;
            push(v);
        }
    }
    /* FIXME: pushing everything takes O(V log V) time; we can sort the
              queue array faster than that by using our knowledge that
              all progress measures are either zero or top. */
}

MaxMeasureLiftingStrategy::~MaxMeasureLiftingStrategy()
{
    delete[] insert_id_;
    delete[] pq_pos_;
    delete[] pq_;
}

void MaxMeasureLiftingStrategy::move_up(verti i)
{
    // FIXME: this can be implemented with less swapping if I think harder.
    for (verti  j; i > 0 && cmp(i, j = (i - 1)/2) > 0; i = j) swap(i, j);
}

void MaxMeasureLiftingStrategy::move_down(verti i)
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

void MaxMeasureLiftingStrategy::swap(verti i, verti j)
{
    verti v = pq_[i], w = pq_[j];
    pq_[i] = w;
    pq_[j] = v;
    pq_pos_[w] = i;
    pq_pos_[v] = j;
}

void MaxMeasureLiftingStrategy::push(verti v)
{
    verti i = pq_pos_[v];
    if (i == (verti)-1)
    {
        i = pq_size_++;
        pq_[i] = v;
        pq_pos_[v] = i;
        if (insert_id_) insert_id_[v] = next_id_++;
    }
    move_up(i);
}
/*
void MaxMeasureLiftingStrategy::remove(verti v)
{
    verti i = pq_pos_[v];
    if (i != (verti)-1)
    {
        pq_pos_[v] = (verti)-1;
        if (i < --pq_size_)
        {
            pq_[i] = pq_[pq_size_];
            pq_pos_[pq_[i]] = i;
            move_down(i);
        }
    }
}
*/
void MaxMeasureLiftingStrategy::pop()
{
    assert(pq_size_ > 0);
    pq_pos_[pq_[0]] = (verti)-1;
    if (0 < --pq_size_)
    {
        pq_[0] = pq_[pq_size_];
        pq_pos_[pq_[0]] = 0;
        move_down(0);
    }
}

int MaxMeasureLiftingStrategy::cmp(verti i, verti j)
{
    verti v = pq_[i], w = pq_[j];
    int d = 0; // spm_.vector_cmp(v, w, spm_.len_);

    if (d == 0 && insert_id_ != NULL)
    {
        // Tie-break on insertion order: smallest insert-id first in queue
        // mode, or largest insert-id first in stack mode.
        compat_uint64_t x = insert_id_[v], y = insert_id_[w];
        d = (x > y) - (x < y);
        if (order_ == STACK) d = -d;
    }
    return d;
}

bool MaxMeasureLiftingStrategy::check()
{
    for (verti i = 1; i < pq_size_; ++i)
    {
        if (cmp(i, (i - 1)/2) > 0) return false;
    }

    for (verti i = 0; i < pq_size_; ++i)
    {
        if (pq_pos_[pq_[i]] != i) return false;
    }

    for (verti v = 0; v < graph_.V(); ++v)
    {
        if (pq_pos_[v] != (verti)-1)
        {
            if (pq_[pq_pos_[v]] != v) return false;
        }
    }

    return true;
}

void MaxMeasureLiftingStrategy::lifted(verti v)
{
    // Queue predecessors with measure less than top:
    for ( StaticGraph::const_iterator it = graph_.pred_begin(v);
          it != graph_.pred_end(v); ++it )
    {
        // TODO: update successor
        push(*it);
    }
}

verti MaxMeasureLiftingStrategy::next()
{
    assert(check());  // debug
    verti v = top();
    pop();
    assert(check());  // debug
    return v;
}

LiftingStrategy *MaxMeasureLiftingStrategyFactory::create(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    return new MaxMeasureLiftingStrategy(game, spm, backward_, order_);
}
