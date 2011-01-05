// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "MpiRecursiveSolver.h"
#include "GamePartition.h"
#include <mpi.h>
#include <assert.h>
#include <algorithm>
#include <set>
#include <utility>
#include <vector>

/*
#include <sstream>

// For debugging: returns the internal vertex set of the partition as a string.
static std::string str(const GamePartition &part)
{
    std::ostringstream os;
    os << "{ ";
    for ( GamePartition::const_iterator it = part.begin();
          it != part.end(); ++it )
    {
        if (it != part.begin()) os << ", ";
        os << part.global(*it);
    }
    os << " }";
    return os.str();
}
*/

/*! Returns a list of indices at which `incl' is zero. */
static std::vector<verti> collect_complement(std::vector<char> &incl)
{
    std::vector<verti> res;
    for (size_t i = 0; i < incl.size(); ++i)
    {
        if (!incl[i]) res.push_back((verti)i);
    }
    return res;
}

//! Returns the sum of local values of all MPI processes:
static int mpi_sum(int local_value)
{
    int global_sum = 0;
    MPI::COMM_WORLD.Allreduce(&local_value, &global_sum, 1, MPI_INT, MPI_SUM);
    return global_sum;
}

//! Returns whether `local_value' is true in any of the MPI processes:
static bool mpi_or(bool local_value)
{
    return mpi_sum((int)local_value) != 0;
}

//! Returns whether `local_value' is true in all of the MPI processes:
static bool mpi_and(bool local_value)
{
    return mpi_sum((int)!local_value) == 0;
}

MpiRecursiveSolver::MpiRecursiveSolver(const ParityGame &game)
    : ParityGameSolver(game)
{
    // Ensure mpi_rank and mpi_size have been initialized:
    assert(mpi_size > 0 && mpi_rank >= 0 && mpi_rank < mpi_size);

    // Sanity check: we assume vertex indices are unsigned integers:
    assert(sizeof(int) == sizeof(verti));
}

MpiRecursiveSolver::~MpiRecursiveSolver()
{
}

ParityGame::Strategy MpiRecursiveSolver::solve()
{
    const verti V = game().graph().V();

    // Initialize stragegy
    strategy_ = ParityGame::Strategy(V, NO_VERTEX);

    {
        // Select vertices for my initial partition of the game graph:
        std::vector<verti> verts;
        verts.reserve((V + mpi_size - 1)/mpi_size);
        for (verti v = mpi_rank; v < V; v += mpi_size)
        {
            assert(worker(v) == mpi_rank);
            verts.push_back(v);
        }

        // Solve the game:
        GamePartition partition(game(), verts);
        solve(partition, 0);
    }

    // Collect resulting strategy
    ParityGame::Strategy result;
    result.swap(strategy_);
    if (aborted())
    {
        result.clear();
    }
    else
    if (mpi_rank == 0)
    {
        info("Combining strategy...");
        for (verti v = 0; v < V; ++v)
        {
            int i = worker(v);
            if (i != mpi_rank)
            {
                int val = -1;
                MPI::COMM_WORLD.Recv(&val, 1, MPI_INT, i, 0);
                result[v] = (verti)val;
            }
        }
    }
    else  // mpi_rank > 0
    {
        for (verti v = 0; v < V; ++v)
        {
            if (worker(v) == mpi_rank)
            {
                int val = result[v];
                MPI::COMM_WORLD.Send(&val, 1, MPI_INT, 0, 0);
            }
        }
        result.clear();
    }

    return result;
}

void MpiRecursiveSolver::solve(GamePartition &part, int min_prio)
{
    const ParityGame::Player player   = (ParityGame::Player)(min_prio%2);
    const ParityGame::Player opponent = (ParityGame::Player)(1 - min_prio%2);

    assert(min_prio < part.game().d());

    if (part.game().d() - min_prio == 1)
    {
        //Logger::debug("part=%s min_prio=%d", str(part).c_str(), min_prio);

        // Only one priority left; construct trivial strategy.
        for ( GamePartition::const_iterator it = part.begin();
              it != part.end(); ++it )
        {
            const verti v = part.global(*it);
            if (game_.player(v) == player)
            {
                // I win; pick arbitrary successor inside the subgame
                strategy_[v] = part.global(*part.game().graph().succ_begin(*it));
            }
            else
            {
                    // Opponent loses
                strategy_[v] = NO_VERTEX;
            }
        }
        return;
    }

    do
    {
        //Logger::debug("part=%s min_prio=%d", str(part).c_str(), min_prio);

        const verti V = part.total_size();
        if (mpi_rank == 0 && aborted())
        {
            MPI::COMM_WORLD.Abort(1);
            return;
        }

        // Find attractor set of vertices with minimum priority
        std::vector<char> min_prio_attr(V, 0);
        std::vector<verti> min_prio_attr_queue;
        for (verti v = 0; v < V; ++v )
        {
            if (part.game().priority(v) == min_prio)
            {
                min_prio_attr[v] = 1;
                min_prio_attr_queue.push_back(v);
            }
        }
        //debug("|min_prio|=%d", mpi_sum((int)min_prio_attr_queue.size()));
        make_attractor_set(part, player, min_prio_attr, min_prio_attr_queue, true);
        //debug("|min_prio_attr|=%d", mpi_sum((int)set_size(part, min_prio_attr)));
        std::vector<verti> unsolved = collect_complement(min_prio_attr);

        // Check if attractor set covers the entire game:
        if (mpi_and(unsolved.empty())) break;

        // Solve subgame with remaining vertices and fewer priorities:
        GamePartition subpart(part, unsolved);
        solve(subpart, min_prio + 1);

        // Find attractor set of vertices lost to opponent in subgame:
        std::vector<char> lost_attr(V, 0);
        std::vector<verti> lost_attr_queue;
        for ( GamePartition::const_iterator it = subpart.begin();
              it != subpart.end(); ++it )
        {
            verti w = subpart.global(*it);
            assert(game_.winner(strategy_, w) == opponent);
            verti v = part.local(w);
            lost_attr[v] = 1;
            lost_attr_queue.push_back(v);
        }
        //debug("|lost|=%d", (int)lost_attr_queue.size());

        // Check if opponent's winning set is empty:
        if (mpi_and(lost_attr_queue.empty())) break;

        // Create subgame with vertices not yet lost to opponent:
        make_attractor_set(part, opponent, lost_attr, lost_attr_queue);
        //debug("|lost_attr|=%d", (int)set_size(part, lost_attr));

        std::vector<verti> not_lost = collect_complement(lost_attr);
        GamePartition(part, not_lost).swap(part);

    } while (mpi_or(!part.empty()));

    // If we get here, then the opponent's winning set was empty; the strategy
    // for most vertices has already been initialized, except for those with
    // minimum priority. Since the whole game is won by the current player, it
    // suffices to pick an arbitrary successor for these vertices:
    for (GamePartition::const_iterator it = part.begin(); it != part.end(); ++it)
    {
        const verti v = part.global(*it);
        if (game_.priority(v) == min_prio)
        {
            if (game_.player(v) == player)  // player wins
            {
                strategy_[v]
                    = part.global(*part.game().graph().succ_begin(*it));
            }
            else  // opponent loses
            {
                strategy_[v] = NO_VERTEX;
            }
        }
        else
        {
            assert(game_.priority(v) >= min_prio);
            assert(game_.winner(strategy_, v) == player);
        }
    }
}

void MpiRecursiveSolver::mpi_exchange_queues(
    const GamePartition &part, const std::vector<verti> &queue,
    std::vector<char> &attr, std::vector<verti> &next_queue )
{
    // FIXME: this method can probably be optimized somehow.

    const StaticGraph &graph = part.game().graph();
    for (int i = 0; i < mpi_size; ++i)
    {
        for (int j = 0; j < mpi_size; ++j)
        {
            if (i == mpi_rank && j != mpi_rank)
            {
                // Send relevant vertices to j'th process
                for (std::vector<verti>::const_iterator it = queue.begin();
                     it != queue.end(); ++it)
                {
                    assert(attr[*it]);
                    bool found = false;
                    for (StaticGraph::const_iterator jt = graph.pred_begin(*it);
                         !found && jt != graph.pred_end(*it); ++jt)
                    {
                        if (worker(part.global(*jt)) == j) found = true;
                    }
                    for (StaticGraph::const_iterator jt = graph.succ_begin(*it);
                         !found && jt != graph.succ_end(*it); ++jt)
                    {
                        if (worker(part.global(*jt)) == j) found = true;
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

void MpiRecursiveSolver::make_attractor_set(
    const GamePartition &part, ParityGame::Player player,
    std::vector<char> &attr, std::vector<verti> &queue,
    bool quick_start )
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

                if (attr[v]) continue;

                // FIXME: this is a bit of a hack! The intent is to process
                //        internal vertices only, but `part' doesn't store
                //        any info to quickly decide this.
                if (worker(part.global(v)) != mpi_rank) continue;

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
        std::vector<verti> next_queue;
        mpi_exchange_queues(part, queue, attr, next_queue);
        next_queue.swap(queue);
        //debug("next queue size: %d", queue.size());
        local_begin = queue.size();
    }
}

ParityGameSolver *MpiRecursiveSolverFactory::create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size )
{
    (void)vertex_map;       // unused
    (void)vertex_map_size;  // unused

    return new MpiRecursiveSolver(game);
}
