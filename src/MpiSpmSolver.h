// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MPI_SPM_SOLVER_H_INCLUDED
#define MPI_SPM_SOLVER_H_INCLUDED

#include "SmallProgressMeasures.h"
#include "VertexPartition.h"
#include "GamePartition.h"
#include "Logger.h"
#include <mpi.h>

extern int mpi_rank, mpi_size;  // defined and initialized in main.cc

/*! A parity game solver based on Marcin Jurdzinski's small progress measures
    algorithm, with pluggable lifting heuristics. Implements the two-way solving
    approach due to Friedmann, and allows distributed computation using MPI. */
class MpiSpmSolver : public ParityGameSolver, public virtual Logger
{
public:
    MpiSpmSolver( const GamePartition &part,
                  LiftingStrategyFactory *lsf,
                  LiftingStatistics *stats = 0,
                  const verti *vertex_map = 0,
                  verti vertex_map_size = 0 );

    ~MpiSpmSolver();

    ParityGame::Strategy solve();

private:
    MpiSpmSolver(const MpiSpmSolver&);
    MpiSpmSolver &operator=(const MpiSpmSolver&);

protected:
    const GamePartition     &part_;     //!< the game partition being solved
    LiftingStrategyFactory  *lsf_;      //!< used to create lifting strategies
    LiftingStatistics       *stats_;    //!< object to record lifting statistics
    const verti             *vmap_;     //!< current vertex map
    const verti             vmap_size_; //!< size of vertex map
};

class MpiSpmSolverFactory : public ParityGameSolverFactory
{
public:
    MpiSpmSolverFactory( LiftingStrategyFactory *lsf,
                         const VertexPartition *vpart,
                         LiftingStatistics *stats = 0 );
    ~MpiSpmSolverFactory();

    ParityGameSolver *create( const ParityGame &game,
                              const verti *vertex_map,
                              verti vertex_map_size );

private:
    LiftingStrategyFactory  *lsf_;
    const VertexPartition   *vpart_;
    LiftingStatistics       *stats_;
};

#endif /* ndef MPI_SPM_SOLVER_H_INCLUDED */
