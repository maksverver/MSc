// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "MpiRecursiveSolver.h"
#include <mpi.h>
#include <assert.h>
#include <algorithm>
#include <set>
#include <utility>
#include <vector>
#include <sstream>  // debug

// TODO: make GamePartition's members private

/* A game partition is a subgame induced by a starting set of internal vertices
   extended with all vertices that share an edge with an internal vertex.

   In addition to storing the subgame for the extended vertex set, the partition
   stores a mapping of local to global and global to local vertex indices, and
   the local indices of the internal vertices.
*/
class GamePartition
{
public:
    /*! Construct a partition from a global game and an internal vertex set. */
    GamePartition(const ParityGame &old_game, const std::vector<verti> &intern)
    {
        // We assume `intern' is sorted and therefore `internal' will
        // be sorted too. This makes it easier to create subpartitions later.
        assert(is_sorted(intern.begin(), intern.end(), std::less<verti>()));

        std::vector<verti> verts(intern.begin(), intern.end());

        // Find vertices incident to internal vertices
        for (std::vector<verti>::const_iterator it = intern.begin();
             it != intern.end(); ++it)
        {
            // Add predecessors of internal vertices
            for (StaticGraph::const_iterator jt = old_game.graph().pred_begin(*it);
                 jt != old_game.graph().pred_end(*it); ++jt) verts.push_back(*jt);

            // Add successors of internal vertices
            for (StaticGraph::const_iterator jt = old_game.graph().succ_begin(*it);
                 jt != old_game.graph().succ_end(*it); ++jt) verts.push_back(*jt);
        }

        // Make vertex set unique
        std::sort(verts.begin(), verts.end());
        verts.erase(std::unique(verts.begin(), verts.end()), verts.end());

        // Create game
        game.make_subgame(old_game, verts.begin(), verts.end());

        // Create vertex index maps
        global = verts;
        for (verti v = 0; v < (verti)global.size(); ++v) local[global[v]] = v;
        internal = intern;
        for ( std::vector<verti>::iterator it = internal.begin();
              it != internal.end(); ++it ) *it = local[*it];

        // FIXME?  We are currently storing successor/predecessor edges for
        //         external vertices too, but these are never used!
    }

    /*! Constructs a partition as the intersection of an existing partition with
        a vertex subset. */
    GamePartition(const GamePartition &part, const std::vector<verti> &verts)
    {
        std::set_intersection( verts.begin(), verts.end(),
                               part.internal.begin(), part.internal.end(),
                               std::back_inserter(internal) );

        // FIXME: we need to remove vertices that do not have any edges incident
        //        to the internal vertex set, but this is a bit ugly:
        const StaticGraph &g = part.game.graph();
        HASH_SET(verti) used(internal.begin(), internal.end());
        std::vector<verti> new_verts;
        for (std::vector<verti>::const_iterator it = verts.begin();
                it != verts.end(); ++it)
        {
            verti v = *it;
            bool found = used.find(v) != used.end();
            for (StaticGraph::const_iterator it = g.succ_begin(v);
                    !found && it != g.succ_end(v); ++it)
            {
                if (used.find(*it) != used.end()) found = true;
            }
            for (StaticGraph::const_iterator it = g.pred_begin(v);
                    !found && it != g.pred_end(v); ++it)
            {
                if (used.find(*it) != used.end()) found = true;
            }
            if (found) new_verts.push_back(v);
        }

        game.make_subgame(part.game, new_verts.begin(), new_verts.end());
        global.resize(new_verts.size());
        for (verti i = 0; i < (verti)new_verts.size(); ++i)
        {
            global[i] = part.global[new_verts[i]];
            local[global[i]] = i;
        }

        // Map internal vertices to new local indices.
        // FIXME: is there a more efficient way to do this?
        for ( std::vector<verti>::iterator it = internal.begin();
              it != internal.end(); ++it ) *it = local[part.global[*it]];

        // DEBUG: check consistency of vertex index mapping
        /*
        for (verti i = 0; i < (verti)new_verts.size(); ++i)
        {
            assert(local[global[i]] == i);
        }
        for (HASH_MAP(verti, verti)::const_iterator it = local.begin();
             it != local.end(); ++it)
        {
            assert(global[it->second] == it->first);
        }
        */
    }

    ParityGame game;                 //! Local subgame
    std::vector<verti> internal;     //! Local indices of internal vertex set
    std::vector<verti> global;       //! Local to global vertex index map
    HASH_MAP(verti, verti) local;    //! Global to local vertex index map
};

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

    // Create my initial partition of the game graph:
    std::vector<verti> verts;
    verts.reserve((V + mpi_size - 1)/mpi_size);
    for (verti v = mpi_rank; v < V; v += mpi_size)
    {
        assert(worker(v) == mpi_rank);
        verts.push_back(v);
    }
    GamePartition partition(game(), verts);
    solve(partition, 0);

    if (mpi_rank == 0)
    {
        debug("Combining strategy...");
        for (verti v = 0; v < V; ++v)
        {
            int i = worker(v);
            if (i != mpi_rank)
            {
                int val = -1;
                MPI::Status status;
                MPI::COMM_WORLD.Recv(&val, 1, MPI_INT, i, 0, status);
                strategy_[v] = (verti)val;
            }
        }
    }
    else
    {
        for (verti v = 0; v < V; ++v)
        {
            if (worker(v) == mpi_rank)
            {
                int val = strategy_[v];
                MPI::COMM_WORLD.Send(&val, 1, MPI_INT, 0, 0);
            }
        }

        // Clear strategy
        ParityGame::Strategy empty;
        strategy_.swap(empty);
    }
    return strategy_;
}

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

/*! Counts how many of the internal vertices of the given game partition are
    in the set described by `incl' (a binary vector). Used for debugging. */
static size_t set_size(const GamePartition &part, const std::vector<char> &incl)
{
    size_t size = 0;
    for (std::vector<verti>::const_iterator it = part.internal.begin();
         it != part.internal.end(); ++it) if (incl[*it]) ++size;
    return size;
}

//! Returns the sum of local values of all MPI processes:
static int mpi_sum(int local_value)
{
    int global_sum = 0;
    MPI::COMM_WORLD.Allreduce(&local_value, &global_sum, 1, MPI_INT, MPI_SUM);
    return global_sum;
}

void MpiRecursiveSolver::solve(const GamePartition &part, int min_prio)
{
    // TODO: support aborting

    const verti V = part.game.graph().V();
    const ParityGame::Player player   = (ParityGame::Player)(min_prio%2);
    const ParityGame::Player opponent = (ParityGame::Player)(1 - min_prio%2);

    assert(min_prio < part.game.d());

    //debug("enter V=%d min_prio=%d", mpi_sum((int)part.internal.size()), min_prio);

    // Debug-print vertices in game partition:
    /*
    std::ostringstream oss_all;
    std::ostringstream oss_intern;
    for (verti v = 0; v < V; ++v) oss_all << ' ' << part.global[v];
    for (std::vector<verti>::const_iterator it = part.internal.begin();
         it != part.internal.end(); ++it) oss_intern << ' ' << part.global[*it];
    debug("all: %s  internal: %s", oss_all.str().c_str(), oss_intern.str().c_str());
    */

    // For debugging: sanity-check game partition:
    /*
    assert(part.game.graph().V() == part.global.size());
    for (verti v = 0; v < V; ++v)
    {
        assert(part.game.priority(v) >= min_prio);
        assert(part.game.priority(v) == game_.priority(part.global[v]));
        assert(part.game.player(v)   == game_.player(part.global[v]));
    }
    */

    if (part.game.d() - min_prio == 1)
    {
        // Only one priority left; construct trivial strategy.
        for ( std::vector<verti>::const_iterator it = part.internal.begin();
              it != part.internal.end(); ++it )
        {
            const verti v = part.global[*it];
            if (game_.player(v) == player)
            {
                // I win; pick arbitrary successor inside the subgame
                strategy_[v] = part.global[*part.game.graph().succ_begin(*it)];
            }
            else
            {
                    // Opponent loses
                strategy_[v] = NO_VERTEX;
            }
        }
        //debug("leave1 V=%d min_prio=%d", mpi_sum((int)part.internal.size()), min_prio);
        return;
    }

    // Find attractor set of vertices with minimum priority
    std::vector<char> min_prio_attr(V, 0);
    std::vector<verti> min_prio_attr_queue;
    for ( std::vector<verti>::const_iterator it = part.internal.begin();
          it != part.internal.end(); ++it )
    {
        if (part.game.priority(*it) == min_prio)
        {
            min_prio_attr[*it] = 1;
            min_prio_attr_queue.push_back(*it);
        }
    }
    // TODO: Optimization: since priorities of external vertices are known
    // locally, we can add them to the set if necessary without exchanging
    // data with other processes (compare with lost_attr below) but this
    // requires a change to make_attractor_set() too.
    debug("|min_prio|=%d", mpi_sum((int)min_prio_attr_queue.size()));
    make_attractor_set(part, player, min_prio_attr, min_prio_attr_queue);
    debug("|min_prio_attr|=%d", mpi_sum((int)set_size(part, min_prio_attr)));

    std::vector<verti> unsolved = collect_complement(min_prio_attr);
    if (mpi_sum(int(!unsolved.empty())) != 0)
    {
        // Solve subgame with remaining vertices and fewer priorities:
        GamePartition subpart(part, unsolved);
        solve(subpart, min_prio + 1);

        // Find attractor set of vertices lost to opponent in subgame:
        std::vector<char> lost_attr(V, 0);
        std::vector<verti> lost_attr_queue;
        for ( std::vector<verti>::const_iterator it = subpart.internal.begin();
              it != subpart.internal.end(); ++it )
        {
            if (game_.winner(strategy_, subpart.global[*it]) == opponent)
            {
                HASH_MAP(verti, verti)::const_iterator
                    jt = part.local.find(subpart.global[*it]);
                assert(jt != part.local.end());
                verti v = jt->second;
                lost_attr[v] = 1;
                lost_attr_queue.push_back(v);
            }
        }

        // Check if opponent's winning set is non-empty:
        if (mpi_sum(int(!lost_attr_queue.empty())) != 0)
        {
            // Create subgame with vertices not yet lost to opponent:
            debug("|lost|=%d", mpi_sum((int)lost_attr_queue.size()));
            make_attractor_set(part, opponent, lost_attr, lost_attr_queue);
            debug("|lost_attr|=%d", mpi_sum((int)set_size(part, lost_attr)));

            std::vector<verti> not_lost = collect_complement(lost_attr);
            if (mpi_sum(int(!not_lost.empty())) != 0)
            {
                // TODO: implement tail recursion here, to limit the recursion
                //       depth to game.d() and to reduce memory use.
                GamePartition subpart(part, not_lost);
                solve(subpart, min_prio);
                //debug("leave2 V=%d min_prio=%d", mpi_sum((int)part.internal.size()), min_prio);
                return;
            }
            else
            {
                // All vertices are lost. Don't recurse.
                //debug("leave3 V=%d min_prio=%d", mpi_sum((int)part.internal.size()), min_prio);
                return;
            }
        }
    }

    // If we get here, then the opponent's winning set was empty; the strategy
    // for most vertices has already been initialized, except for those with
    // minimum priority. Since the whole game is won by the current player, it
    // suffices to pick an arbitrary successor for these vertices:
    for ( std::vector<verti>::const_iterator it = part.internal.begin();
        it != part.internal.end(); ++it )
    {
        const verti v = part.global[*it];
        if (game_.priority(v) == min_prio)
        {
            if (game_.player(v) == player)  // player wins
            {
                strategy_[v]
                    = part.global[*part.game.graph().succ_begin(*it)];
            }
            else  // opponent loses
            {
                strategy_[v] = NO_VERTEX;
            }
        }
        else
        {
            assert( game_.priority(v) >= min_prio );
            assert( (game_.player(v) == player) ==
                    (strategy_[v] != NO_VERTEX) );
        }
    }

    //debug("leave4 V=%d min_prio=%d", mpi_sum((int)part.internal.size()), min_prio);
}

void MpiRecursiveSolver::mpi_exchange_queues(
    const GamePartition &part, const std::vector<verti> &queue,
    std::vector<char> &attr, std::vector<verti> &next_queue )
{
    // FIXME: this method can probably be optimized somehow.

    const StaticGraph &graph = part.game.graph();
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
                        if (worker(part.global[*jt]) == j) found = true;
                    }
                    for (StaticGraph::const_iterator jt = graph.succ_begin(*it);
                         !found && jt != graph.succ_end(*it); ++jt)
                    {
                        if (worker(part.global[*jt]) == j) found = true;
                    }
                    if (found)
                    {
                        int val = (int)part.global[*it];
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
                    MPI::Status status;
                    MPI::COMM_WORLD.Recv(&val, 1, MPI_INT, i, 0, status);
                    if (val == -1) break;
                    //debug("received %d from %d", val, i);
                    HASH_MAP(verti, verti)::const_iterator
                        it = part.local.find(val);
                    assert(it != part.local.end());
                    const verti v = it->second;
                    assert(!attr[v]);
                    attr[v] = 1;
                    next_queue.push_back(it->second);
                }
            }
        }
    }
}

void MpiRecursiveSolver::make_attractor_set(
    const GamePartition &part, ParityGame::Player player,
    std::vector<char> &attr, std::vector<verti> &queue )
{
    // Offset into `queue' where local entries (internal vertices) begin:
    size_t local_begin = 0;
    while (mpi_sum((int)queue.size()) > 0)
    {
        // Calculate maximal internal attractor set
        for (size_t pos = 0; pos < queue.size(); ++pos)
        {
            const StaticGraph &graph = part.game.graph();
            const verti w = queue[pos];
            for ( StaticGraph::const_iterator it = graph.pred_begin(w);
                  it != graph.pred_end(w); ++it )
            {
                const verti v = *it;

                if (attr[v]) continue;

                // FIXME: this is a bit of a hack! The intent is to process
                //        internal vertices only, but `part' doesn't store
                //        any info to quickly decide this.
                if (worker(part.global[v]) != mpi_rank) continue;

                if (part.game.player(v) == player)
                {
                    // Store strategy for player-controlled vertex:
                    strategy_[part.global[v]] = part.global[w];
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
                    strategy_[part.global[v]] = NO_VERTEX;
                }
                // Add vertex v to the attractor set:
                attr[v] = true;
                queue.push_back(v);
                //debug("added %d to attractor set", part.global[v]);

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
