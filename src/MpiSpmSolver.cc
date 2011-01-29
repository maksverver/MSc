// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "MpiSpmSolver.h"

MpiSpmSolver::MpiSpmSolver( const GamePartition &part,
        LiftingStrategyFactory *lsf, LiftingStatistics *stats,
        const verti *vertex_map, verti vertex_map_size )
    : ParityGameSolver(part.game()), part_(part), lsf_(lsf), stats_(stats),
      vmap_(vertex_map), vmap_size_(vertex_map_size)
{
    lsf_->ref();
}

MpiSpmSolver::~MpiSpmSolver()
{
    lsf_->deref();
}

ParityGame::Strategy MpiSpmSolver::solve()
{
    

    // TODO: implement two-way approach due to Friedmann.
    // TODO: merge lifting statistics
    // TODO: merge resulting strategy
    ParityGame::Strategy result;

    return result;
}

MpiSpmSolverFactory::MpiSpmSolverFactory( LiftingStrategyFactory *lsf,
        const VertexPartition *vpart, LiftingStatistics *stats )
    : lsf_(lsf), vpart_(vpart), stats_(stats)
{
    vpart_->ref();
    lsf_->ref();
}

MpiSpmSolverFactory::~MpiSpmSolverFactory()
{
    lsf_->deref();
    vpart_->deref();
}

ParityGameSolver *MpiSpmSolverFactory::create( const ParityGame &game,
    const verti *vertex_map, verti vertex_map_size )
{
    GamePartition part(game, *vpart_, mpi_rank);
    return new MpiSpmSolver( part, lsf_, stats_, vertex_map, vertex_map_size);
}
