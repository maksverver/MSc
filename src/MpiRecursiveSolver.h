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

#include "RecursiveSolver.h"
#include "VertexPartition.h"
#include "GamePartition.h"
#include "MpiAttractorAlgorithm.h"
#include "Logger.h"
#include <vector>
#include <deque>
#include <mpi.h>

extern int mpi_rank, mpi_size;  // defined and initialized in main.cc

/*! Parallel version of RecursiveSolver implemented using MPI.
 
    Must currently be subclassed with a suitable implementation of
    make_attractor_set(). TODO: factor parallel attractor set computation out
    into separate classes that are passed as a parameter to this solver class.
*/
class MpiRecursiveSolver : public ParityGameSolver, public Logger
{
public:
    // N.B. takes ownership of `attr_algo'!
    MpiRecursiveSolver( const ParityGame &game, const VertexPartition &vpart,
                        MpiAttractorAlgorithm *attr_algo );
    ~MpiRecursiveSolver();

    ParityGame::Strategy solve();

protected:
    /*! Solves the game for the internal vertex set of the given game partition
        and updates `strategy_' so that it is valid for all global indices
        corresponding with internal vertices of the partition.

        After returning, the game partition has been reduced to the winning set
        for the player corresponding to the minimum priority used in `part'.
    */
    void solve(GamePartition &part);

protected:
    //! Vertex partition used to partition the game over MPI worker processes.
    VertexPartition vpart_;

    //! Pointer to algorithm used to compute attractor sets using MPI.
    MpiAttractorAlgorithm *attr_algo_;

    /*! Resulting strategy (only valid for my partition of the vertex set).
        Note that the strategy is global; it uses global indices for its indices
        as well as its values. */
    ParityGame::Strategy strategy_;
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
