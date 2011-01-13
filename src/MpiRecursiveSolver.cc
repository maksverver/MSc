// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "AsyncMpiAttractorAlgorithm.h"
#include "SyncMpiAttractorAlgorithm.h"
#include "MpiRecursiveSolver.h"
#include "GamePartition.h"
#include <assert.h>
#include <algorithm>
#include <set>
#include <utility>

//! Returns a list of indices at which `incl' is zero.
static std::vector<verti> collect_complement(std::vector<char> &incl)
{
    std::vector<verti> res;
    for (size_t i = 0; i < incl.size(); ++i)
    {
        if (!incl[i]) res.push_back((verti)i);
    }
    return res;
}

//! Returns the sum of local values of all MPI processes: (used for debugging)
/*
static int mpi_sum(int local_value)
{
    int global_sum = 0;
    MPI::COMM_WORLD.Allreduce(&local_value, &global_sum, 1, MPI_INT, MPI_SUM);
    return global_sum;
}
*/

//! Returns whether `local_value' is true in all of the MPI processes:
static bool mpi_and(int local_value)
{
    int res;
    MPI::COMM_WORLD.Allreduce(&local_value, &res, 1, MPI_INT, MPI_LAND);
    return res;
}

MpiRecursiveSolver::MpiRecursiveSolver( const ParityGame &game,
    const VertexPartition &vpart, MpiAttractorAlgorithm *attr_algo )
    : ParityGameSolver(game), vpart_(vpart), attr_algo_(attr_algo)
{
    // Ensure mpi_rank and mpi_size have been initialized:
    assert(mpi_size > 0 && mpi_rank >= 0 && mpi_rank < mpi_size);

    // Sanity check: we assume vertex indices are unsigned integers:
    assert(sizeof(int) == sizeof(verti));
}

MpiRecursiveSolver::~MpiRecursiveSolver()
{
    delete attr_algo_;
}

ParityGame::Strategy MpiRecursiveSolver::solve()
{
    const verti V = game().graph().V();

    // Initialize stragegy
    strategy_ = ParityGame::Strategy(V, NO_VERTEX);

    // Solve the game:
    GamePartition gpart(game(), vpart_, mpi_rank);
    solve(gpart);

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
            int i = vpart_(v);
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
            if (vpart_(v) == mpi_rank)
            {
                int val = result[v];
                MPI::COMM_WORLD.Send(&val, 1, MPI_INT, 0, 0);
            }
        }
        result.clear();
    }

    return result;
}

/* Returns the first alternation in the game, similar to the function of the
   same name in RecursiveSolver.cc, but this version first collects information
   about priorities used in all MPI worker processes. */
static int mpi_first_alternation(const ParityGame &local_game)
{
    int d = local_game.d();
    unsigned char local_used[256], used[256];  // I wish C++ supported VLA's...

    // Find out which priorities are in use, globally:
    assert(d < 256);
    for (int p = 0; p < d; ++p) local_used[p] = local_game.cardinality(p) > 0;
    MPI_Allreduce(local_used, used, d, MPI::BYTE, MPI::BOR, MPI::COMM_WORLD);

    // Determine where first alternation occurs:
    int q = 0;
    while (q < d && !used[q]) ++q;
    int p = q + 1;
    while (p < d && !used[p]) p += 2;
    if (p > d) p = d;
    return p;
}

void MpiRecursiveSolver::solve(GamePartition &part)
{
    int prio;
    while ((prio = mpi_first_alternation(part.game())) < part.game().d())
    {
        //debug("part=%s prio=%d", part.debug_str().c_str(), prio);

        const verti V = part.total_size();
        if (mpi_rank == 0 && aborted())
        {
            MPI::COMM_WORLD.Abort(1);
            return;
        }

        // Find attractor set of vertices with minimum priority
        std::vector<char> min_prio_attr(V, 0);
        std::deque<verti> min_prio_attr_queue;
        for (verti v = 0; v < V; ++v )
        {
            if (part.game().priority(v) < prio)
            {
                min_prio_attr[v] = 1;
                min_prio_attr_queue.push_back(v);
            }
        }
        attr_algo_->make_attractor_set(
            vpart_, part, (ParityGame::Player)((prio - 1)%2),
            min_prio_attr, min_prio_attr_queue, true, strategy_ );
        std::vector<verti> unsolved = collect_complement(min_prio_attr);

        // Check if attractor set covers the entire game:
        if (mpi_and(unsolved.empty())) break;

        // Solve subgame with remaining vertices and fewer priorities:
        GamePartition subpart(part, unsolved);
        solve(subpart);

        // Find attractor set of vertices lost to opponent in subgame:
        std::vector<char> lost_attr(V, 0);
        std::deque<verti> lost_attr_queue;
        for ( GamePartition::const_iterator it = part.begin();
               it != part.end(); ++it )
        {
            const verti v = *it;
            if ( !min_prio_attr[v] &&
                 game().winner(strategy_, part.global(v)) == prio%2 )
            {
                lost_attr[v] = 1;
                lost_attr_queue.push_back(v);
            }
        }

        // Check if opponent's winning set is empty:
        if (mpi_and(lost_attr_queue.empty())) break;

        // Create subgame with vertices not yet lost to opponent:
        attr_algo_->make_attractor_set(
            vpart_, part, (ParityGame::Player)(prio%2),
            lost_attr, lost_attr_queue, false, strategy_ );

        std::vector<verti> not_lost = collect_complement(lost_attr);
        GamePartition(part, not_lost).swap(part);
    }

    // If we get here, then the opponent's winning set was empty; the strategy
    // for most vertices has already been initialized, except for those with
    // minimum priority. Since the whole game is won by the current player, it
    // suffices to pick an arbitrary successor for these vertices:
    for (GamePartition::const_iterator it = part.begin(); it != part.end(); ++it)
    {
        const verti v = part.global(*it);
        if (game_.priority(v) < prio)
        {
            if (game_.player(v) == game_.priority(v)%2)  // player wins
            {
                strategy_[v] = part.global(*part.game().graph().succ_begin(*it));
            }
            else  // opponent loses
            {
                strategy_[v] = NO_VERTEX;
            }
        }
    }
}

ParityGameSolver *MpiRecursiveSolverFactory::create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size )
{
    (void)vertex_map;       // unused
    (void)vertex_map_size;  // unused

    VertexPartition vpart(mpi_size, chunk_size_ > 0 ? chunk_size_
        : (game.graph().V() + mpi_size - 1)/mpi_size);

    MpiAttractorAlgorithm *attr_algo;
    if (async_)
    {
        attr_algo = new AsyncMpiAttractorAlgorithm;
    }
    else  // !async_
    {
        attr_algo = new SyncMpiAttractorAlgorithm;
    }

    Logger::info(
        "Constructing %s MpiRecursiveSolver with %ld vertices per chunk.",
        async_ ? "asynchronous" : "synchronized", (long)vpart.chunk_size() );

    // N.B. MpiRecursiveSolver takes ownership of `attr_algo'
    return new MpiRecursiveSolver(game, vpart, attr_algo);
}