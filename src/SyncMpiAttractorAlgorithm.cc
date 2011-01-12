// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "SyncMpiAttractorAlgorithm.h"

extern int mpi_rank, mpi_size;

//! Returns whether `local_value' is true in any of the MPI processes:
static bool mpi_or(int local_value)
{
    int res;
    MPI::COMM_WORLD.Allreduce(&local_value, &res, 1, MPI_INT, MPI_LOR);
    return res;
}

SyncMpiAttractorImpl::SyncMpiAttractorImpl( const VertexPartition &vpart,
        const GamePartition &part, ParityGame::Player player,
        std::vector<char> &attr, std::deque<verti> &queue,
        ParityGame::Strategy &strategy )
    : vpart_(vpart), part(part), player(player), attr(attr), queue(queue),
      strategy_(strategy)
{
}

void SyncMpiAttractorImpl::solve(bool quick_start)
{
    // Offset into `queue' where local entries (internal vertices) begin:
    size_t local_begin = quick_start ? queue.size() :  0;
    while (mpi_or(!queue.empty()))
    {
        // Calculate maximal internal attractor set
        for (size_t pos = 0; pos < queue.size(); ++pos)
        {
            const StaticGraph &graph = part.game().graph();
            const verti w = queue[pos];
            for ( StaticGraph::const_iterator it = graph.pred_begin(w);
                  it != graph.pred_end(w); ++it )
            {
                const verti v = *it;

                // Skip vertices already in the attractor set:
                if (attr[v]) continue;

                // Skip vertices not assigned to this worker process:
                if (vpart_(part.global(v)) != mpi_rank) continue;

                if (part.game().player(v) == player)
                {
                    // Store strategy for player-controlled vertex:
                    strategy_[part.global(v)] = part.global(w);
                }
                else  // opponent-controlled vertex
                {
                    // Can the opponent keep the token out of the attractor set?
                    for (StaticGraph::const_iterator jt = graph.succ_begin(v);
                        jt != graph.succ_end(v); ++jt)
                    {
                        if (!attr[*jt]) goto skip_v;
                    }

                    // Store strategy for opponent-controlled vertex:
                    strategy_[part.global(v)] = NO_VERTEX;
                }
                // Add vertex v to the attractor set:
                attr[v] = true;
                queue.push_back(v);
                //debug("added %d to attractor set", part.global(v));

            skip_v:
                continue;
            }
        }
        // Synchronize with other processes, obtaining a fresh queue of
        // external vertices that were added in parallel:
        queue.erase(queue.begin(), queue.begin() + local_begin);
        //debug("queue size: %d", queue.size());
        std::deque<verti> next_queue;
        exchange_queues(next_queue);
        next_queue.swap(queue);
        //debug("next queue size: %d", queue.size());
        local_begin = queue.size();
    }
}

void SyncMpiAttractorImpl::exchange_queues(std::deque<verti> &next_queue)
{
    const StaticGraph &graph = part.game().graph();
    for (int i = 0; i < mpi_size; ++i)
    {
        for (int j = 0; j < mpi_size; ++j)
        {
            if (i == mpi_rank && j != mpi_rank)
            {
                // Send relevant vertices to j'th process
                for (std::deque<verti>::const_iterator it = queue.begin();
                     it != queue.end(); ++it)
                {
                    assert(attr[*it]);
                    bool found = false;
                    for (StaticGraph::const_iterator jt = graph.pred_begin(*it);
                         !found && jt != graph.pred_end(*it); ++jt)
                    {
                        if (vpart_(part.global(*jt)) == j) found = true;
                    }
                    for (StaticGraph::const_iterator jt = graph.succ_begin(*it);
                         !found && jt != graph.succ_end(*it); ++jt)
                    {
                        if (vpart_(part.global(*jt)) == j) found = true;
                    }
                    if (found)
                    {
                        int val = (int)part.global(*it);
                        MPI::COMM_WORLD.Send(&val, 1, MPI_INT, j, 0);
                        //debug("sending %d to %d", val, j);
                    }
                }
                int val = -1;
                MPI::COMM_WORLD.Send(&val, 1, MPI_INT, j, 0);
            }

            if (i != mpi_rank && j == mpi_rank)
            {
                // Receive relevant vertices from i'th process
                for (;;)
                {
                    int val = -1;
                    MPI::COMM_WORLD.Recv(&val, 1, MPI_INT, i, 0);
                    if (val == -1) break;
                    //debug("received %d from %d", val, i);
                    const verti v = part.local((verti)val);
                    assert(!attr[v]);
                    attr[v] = 1;
                    next_queue.push_back(v);
                }
            }
        }
    }
}