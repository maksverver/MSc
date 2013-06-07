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
    bool backward, Order order )
        : LiftingStrategy2(game), spm_(spm), order_(order), next_id_(0),
          insert_id_(order < HEAP ? new compat_uint64_t[graph_.V()] : NULL),
          pq_pos_(new verti[graph_.V()]), pq_(new verti[graph_.V()]),
          pq_size_(0)
{
    std::fill(&pq_pos_[0], &pq_pos_[graph_.V()], NO_VERTEX);
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
    for (verti  j; i > 0 && cmp(i, j = (i - 1)/2) > 0; i = j) swap(i, j);
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
    bump(v);
}

void MaxMeasureLiftingStrategy2::bump(verti v)
{
    bumped_.push_back(v);
    Logger::debug("bump(%d)", v);
/*
    verti i = pq_pos_[v];
    assert(i != NO_VERTEX && pq_[i] == v);
    move_up(i);
    assert(check());  // DEBUG
*/
}
/*
void MaxMeasureLiftingStrategy::remove(verti v)
{
    verti i = pq_pos_[v];
    if (i != NO_VERTEX)
    {
        pq_pos_[v] = NO_VERTEX;
        if (i < --pq_size_)
        {
            pq_[i] = pq_[pq_size_];
            pq_pos_[pq_[i]] = i;
            move_down(i);
        }
    }
}
*/
verti MaxMeasureLiftingStrategy2::pop()
{
    if (!bumped_.empty())
    {
        /*
        std::cerr << "bumped:";
        for (size_t i = 0; i < bumped_.size(); ++i)
            std::cerr << ' ' << bumped_[i];
        std::cerr << std::endl;
        */
        for ( std::vector<verti>::iterator it = bumped_.begin();
              it != bumped_.end(); ++it )
        {
            *it = pq_pos_[*it];
            assert(*it != NO_VERTEX);
        }
        std::sort(bumped_.begin(), bumped_.end());
        bumped_.erase( std::unique(bumped_.begin(), bumped_.end()),
                       bumped_.end() );
        for ( std::vector<verti>::iterator it = bumped_.begin();
              it != bumped_.end(); ++it )
        {
            move_up(*it);
        }
        //assert(check());  // DEBUG
        bumped_.clear();
    }

    if (pq_size_ == 0) return NO_VERTEX;
    verti v = pq_[0];
    pq_pos_[v] = (verti)-1;
    if (--pq_size_ > 0)
    {
        pq_[0] = pq_[pq_size_];
        pq_pos_[pq_[0]] = 0;
        move_down(0);
    }
    //assert(check());  // DEBUG
    Logger::debug("pop() -> %d", v);
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
    if (d != 0) return d;

    // Tie-break on insertion order: smallest insert-id first in queue
    // mode, or largest insert-id first in stack mode.
    switch (order_)
    {
    case STACK: return cmp_ids(insert_id_[v], insert_id_[w]);
    case QUEUE: return cmp_ids(insert_id_[w], insert_id_[v]);
    default:    return d;
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

    for (verti v = 0; v < graph_.V(); ++v)
    {
        if (pq_pos_[v] != (verti)-1)
        {
            if (pq_[pq_pos_[v]] != v) return false;
        }
    }

    return true;
}

LiftingStrategy *MaxMeasureLiftingStrategyFactory::create(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    return 0;
}

LiftingStrategy2 *MaxMeasureLiftingStrategyFactory::create2(
    const ParityGame &game, const SmallProgressMeasures &spm )
{
    return new MaxMeasureLiftingStrategy2(game, spm, backward_, order_);
}
