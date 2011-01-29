// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "MpiSpmSolver.h"

MpiSpmSolver::MpiSpmSolver(
        const ParityGame &game, const VertexPartition *vpart,
        LiftingStrategyFactory *lsf, LiftingStatistics *stats,
        const verti *vertex_map, verti vertex_map_size )
    : ParityGameSolver(game), vpart_(vpart), part_(game, *vpart_, mpi_rank),
      lsf_(lsf), stats_(stats), vmap_(vertex_map), vmap_size_(vertex_map_size)
{
    vpart_->ref();
    lsf_->ref();
}

MpiSpmSolver::~MpiSpmSolver()
{
    vpart_->deref();
    lsf_->deref();
}

void MpiSpmSolver::set_vector_space(SmallProgressMeasures &spm)
{
    std::vector<verti> local(spm.len(), 1);
    const ParityGame &game = part_.game();
    for ( GamePartition::const_iterator it = part_.begin();
          it != part_.end(); ++it )
    {
        verti v = *it;
        if (!spm.is_top(v))
        {
            int prio = game.priority(v);
            if (prio%2 != spm.player()) ++local[prio/2];
        }
    }
    std::vector<verti> global(spm.len());
    MPI::COMM_WORLD.Allreduce( &local[0], &global[0], spm.len(),
                               MPI_INT, MPI_SUM );
    spm.set_M(&global[0]);
}

void MpiSpmSolver::solve_all(SmallProgressMeasures &spm)
{
    // TODO! Similar to recursive solver:
    //  - lift using local queue
    //  - send notifications to neighbours when lifting succeeds
    //  - detect global termination
}

void MpiSpmSolver::propagate_solved( SmallProgressMeasures &src,
                                     SmallProgressMeasures &dst )
{
    assert(src.player() != dst.player());
    SetToTopIterator it = { dst };
    src.get_winning_set(src.player(), it);
}

ParityGame::Strategy MpiSpmSolver::combine_strategies(
    ParityGame::Strategy &local_strategy )
{
    ParityGame::Strategy result;
    const verti V = game_.graph().V();
    if (mpi_rank == 0)
    {
        info("Merging strategy...");
        result.resize(V, NO_VERTEX - 1);
        for (verti v = 0; v < V; ++v)
        {
            int p = (*vpart_)(v);
            if (p == 0)
            {
                verti w = part_.local(v);
                assert(w != NO_VERTEX);
                result[v] = local_strategy[w];
            }
            else
            {
                MPI::COMM_WORLD.Recv(&result[v], 1, MPI_INT, p, 0);
            }
        }
    }
    else
    {
        for (verti v = 0; v < V; ++v)
        {
            verti w = part_.local(v);
            if (w != NO_VERTEX)
            {
                MPI::COMM_WORLD.Send(&local_strategy[w], 1, MPI_INT, 0, 0);
            }
        }
    }
    return result;
}


// TODO: support two-way approach due to Friedmann.
ParityGame::Strategy MpiSpmSolver::solve()
{
    assert(sizeof(verti) == sizeof(int));

    // Create a local statistics object, but only if required globally:
    std::auto_ptr<LiftingStatistics> stats;
    if (stats_) stats.reset(new LiftingStatistics(part_.game()));

    // Create two SPM instances (one for each player):
    std::auto_ptr<SmallProgressMeasures> spm[2];
    /* NOTE: SmallProgressMeasures initializes vertices with just a beneficial
       loop to Top, so all game parts must include edges for external vertices,
       or the progress measures are out of sync! */
    spm[0].reset( new SmallProgressMeasures(
        part_.game(), ParityGame::PLAYER_EVEN, lsf_, stats.get(), NULL, 0 ) );
    spm[1].reset( new SmallProgressMeasures(
        part_.game(), ParityGame::PLAYER_ODD, lsf_, stats.get(), NULL, 0 ) );

    // Solve the two games, one after the other:
    info("Initializing vector space...");
    set_vector_space(*spm[0]);
    info("Solving game for Even...");
    solve_all(*spm[0]);
    info("Propagating winning set to dual game...");
    propagate_solved(*spm[0], *spm[1]);
    info("Initializing vector space...");
    set_vector_space(*spm[1]);
    info("Solving dual game for Odd...");
    solve_all(*spm[1]);
    info("Extracting local strategy...");
    ParityGame::Strategy strategy(part_.game().graph().V(), NO_VERTEX);
    spm[0]->get_strategy(strategy);
    spm[1]->get_strategy(strategy);

    // Combine lifting statistics
    if (stats_)
    {
        const verti V = game_.graph().V();  // N.B. GLOBAL graph size!
        if (mpi_rank == 0)
        {
            info("Merging lifting statistics...");
            // Receive total lift statistics from other workers:
            long long attempted = stats->lifts_attempted();
            long long succeeded = stats->lifts_succeeded();
            long long as[2];
            for (int i = 1; i < mpi_size; ++i)
            {
                MPI::COMM_WORLD.Recv(as, 2, MPI_LONG_LONG, i, 0);
                attempted += as[0];
                succeeded += as[1];
            }
            stats_->add_lifts_attempted(attempted);
            stats_->add_lifts_succeeded(succeeded);

            // Receive per-vertex lifting statistics:
            for (verti v = 0; v < V; ++v)
            {
                int p = (*vpart_)(v);
                if (p == 0)
                {
                    verti w = part_.local(v);
                    assert(w != NO_VERTEX);
                    as[0] = stats->lifts_attempted(w);
                    as[1] = stats->lifts_succeeded(w);
                }
                else
                {
                    MPI::COMM_WORLD.Recv(as, 2, MPI_LONG_LONG, p, 0);
                }

                // Assign statistics, with respect to vertec mapping:
                verti u = v;
                if (vmap_ && v < vmap_size_) u = vmap_[u];
                stats_->add_lifts_attempted(u, as[0]);
                stats_->add_lifts_succeeded(u, as[1]);
            }
        }
        else
        {
            // Send total lift statistics to root:
            long long as[2] = { stats->lifts_attempted(),
                                stats->lifts_succeeded() };
            MPI::COMM_WORLD.Send(as, 2, MPI_LONG_LONG, 0, 0);

            // Send per-vertex lifting statistics:
            for (verti v = 0; v < V; ++v)
            {
                verti w = part_.local(v);
                if (w != NO_VERTEX)
                {
                    as[0] = stats->lifts_attempted(w);
                    as[1] = stats->lifts_attempted(w);
                    MPI::COMM_WORLD.Send(as, 2, MPI_LONG_LONG, 0, 0);
                }
            }
        }
    }

    return combine_strategies(strategy);
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
    return new MpiSpmSolver( game, vpart_, lsf_, stats_,
                             vertex_map, vertex_map_size );
}
