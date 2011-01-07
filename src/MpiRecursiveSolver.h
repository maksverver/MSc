// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MPI_RECURSIVE_SOLVER_H_INCLUDED
#define MPI_RECURSIVE_SOLVER_H_INCLUDED

#include "ParityGameSolver.h"
#include "Logger.h"
#include <vector>
#include <deque>
#include <mpi.h>

extern int mpi_rank, mpi_size;  // defined and initialized in main.cc

class GamePartition;

/*! Parallel version of RecursiveSolver implemented using MPI.
 
    Must currently be subclassed with a suitable implementation of
    make_attractor_set(). TODO: factor parallel attractor set computation out
    into separate classes that are passed as a parameter to this solver class.
*/
class MpiRecursiveSolver : public ParityGameSolver, public Logger
{
public:
    MpiRecursiveSolver(const ParityGame &game);
    ~MpiRecursiveSolver();

    ParityGame::Strategy solve();

protected:
    //! Maps vertices to worker processes
    int worker(verti v) { return (int)(v%(verti)mpi_size); }

    //! Maps vertices to local indices
    verti index(verti v) { return v/(verti)mpi_size; }

protected:
    /*! Solves the game for the internal vertex set of the given game partition,
        given that the minimum priority used in the game is `min_prio'.

        Updates `strategy_' so that it is valid for all global indices
        corresponding with internal vertices of the partition.

        After returning, the game partition has been reduced to the winning set
        for the player corresponding to the parity of min_prio.
    */
    void solve(GamePartition &part, int min_prio);

    /*! Extends the vertices marked in `attr' to the attractor set for `player'
        in the game partition `part'. Initially, `queue' must contain precisely
        those vertices marked in `attr'.

        If `quick_start' is set to true, then `attr' should include all external
        vertices that are in the `attr' in other worker processes. Otherwise,
        `attr' initially only contains internal vertices.

        After returning, the set is extended to the attractor set for `player'
        and includes both internal and external vertices in the attractor set.
    */
    virtual void make_attractor_set(
        const GamePartition &part, ParityGame::Player player,
        std::vector<char> &attr, std::deque<verti> &queue,
        bool quick_start = false ) = 0;

protected:
    /*! Resulting strategy (only valid for my partition of the vertex set).
        Note that the strategy is global; it uses global indices for its indices
        as well as its values. */
    ParityGame::Strategy strategy_;
};

/*! MpiRecursiveSolver implementation that runs the attractor set computation
    asynchronously; i.e. all worker processes send and receive vertices to be
    added to the set while they are running independent breadth-first search
    over their local vertex set. This is should reduce latency. */
class AsyncMpiRecursiveSolver : public MpiRecursiveSolver
{
public:
    //! tags used to identify different types of messages exchanged through MPI.
    enum MpiTags { TAG_VERTEX, TAG_PROBE, TAG_TERM };

    AsyncMpiRecursiveSolver(const ParityGame &game)
        : MpiRecursiveSolver(game)
    {
        Logger::info("Constructed asynchronous recursive solver.");
    }

    void make_attractor_set(
        const GamePartition &part, ParityGame::Player player,
        std::vector<char> &attr, std::deque<verti> &queue, bool quick_start );

private:
    /*! Helper function for make_attractor_set() that transmits `v' to relevant
        other processes, and then receives any pending vertices from other
        processes, which are then added to `queue' and `attr'. When messages
        are sent or received, `num_send' and `num_recv' are incremented.

        This method takes a lot of arguments, but it's intended to be inlined
        into make_attractor_set(), which calls it twice, but without duplicating
        the code manually.
    */
    void notify_others( const GamePartition &part, verti v,
                        std::deque<verti> &queue, std::vector<char> &attr,
                        MPI::Prequest &req, const verti &req_val,
                        int &num_send, int &num_recv );
};

/*! MpiRecursiveSolver implementation that runs the attractor set computation
    synchronously; i.e. all worker processes compute the attractor set in lock-
    step, synchronizing after adding a layer of vertices to the attractor set.
    In every step the vertices that lie one step further from the initial set
    are computed. The downside of this is that a lot synchronization is done
    when there are vertices in the attractor set that lie far away from the
    closest initial vertex, but the advantage is that termination is easy to
    detect, which is why this algorithm was initially implemented. */
class SyncMpiRecursiveSolver : public MpiRecursiveSolver
{
public:
    SyncMpiRecursiveSolver(const ParityGame &game)
        : MpiRecursiveSolver(game)
    {
        Logger::info("Constructed synchronized recursive solver.");
    }

    void make_attractor_set(
        const GamePartition &part, ParityGame::Player player,
        std::vector<char> &attr, std::deque<verti> &queue, bool quick_start );

private:
    /*! Exchanges attractor set queues between MPI worker processes.

        For each worker, `queue' contains internal vertices added to `attr'
        last iteration, which are sent to other workers controlling the
        predecessors of these vertices. The vertices received this way are added
        to `next_queue' and set in `attr'. Consequently, only external vertices
        are added to `next_queue'.
    */
    void mpi_exchange_queues(
        const GamePartition &part, const std::deque<verti> &queue,
        std::vector<char> &attr, std::deque<verti> &next_queue );
};

class MpiRecursiveSolverFactory : public ParityGameSolverFactory
{
public:
    MpiRecursiveSolverFactory(bool async) : async_(async) { };

    ParityGameSolver *create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size );

private:
    bool async_;
};

#endif /* ndef MPI_RECURSIVE_SOLVER_H_INCLUDED */
