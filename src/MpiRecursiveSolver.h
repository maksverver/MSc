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

private:
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
    void make_attractor_set(
        const GamePartition &part, ParityGame::Player player,
        std::vector<char> &attr, std::deque<verti> &queue,
        bool quick_start = false);

    /*! Helper function for make_attractor_set() that transmits `v' to relevant
        other processes, and then receives any pending vertices from other
        processes, which are then added to `queue' and `attr'. When messages
        are sent or received, `num_send' and `num_recv' are incremented. */
    void notify_others( const GamePartition &part, verti v,
                        std::deque<verti> &queue, std::vector<char> &attr, 
                        MPI::Prequest &req, const verti &req_val,
                        int &num_send, int &num_recv );

    /*! Resulting strategy (only valid for my partition of the vertex set).
        Note that the strategy is global; it uses global indices for its indices
        as well as its values. */
    ParityGame::Strategy strategy_;
};

class MpiRecursiveSolverFactory : public ParityGameSolverFactory
{
    ParityGameSolver *create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size );
};

#endif /* ndef MPI_RECURSIVE_SOLVER_H_INCLUDED */
