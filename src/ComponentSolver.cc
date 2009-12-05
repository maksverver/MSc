// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "ComponentSolver.h"
#include "logging.h"

#include <assert.h>
#include <memory>

ComponentSolver::ComponentSolver( const ParityGame &game,
                                  const std::string &lift_strat,
                                  LiftingStatistics *stats )
    : ParityGameSolver(game), lift_strat_(lift_strat),
      stats_(stats), memory_used_(0)
{
}

ComponentSolver::~ComponentSolver()
{
}

ParityGame::Strategy ComponentSolver::solve()
{
    strategy_ = ParityGame::Strategy(game_.graph().V(), NO_VERTEX);

    if (decompose_graph(game_.graph(), *this) != 0)
    {
        error("Component solving failed!");
        strategy_.clear();
    }

    return strategy_;
}

int ComponentSolver::operator()(const verti *vertices, size_t num_vertices)
{
    info("Constructing subgame with %d vertices...", (int)num_vertices);

    // Construct a subgame
    ParityGame subgame;
    subgame.make_subgame(game_, vertices, num_vertices, strategy_);
    size_t mem = subgame.memory_use();

    // Compress vertex priorities
    int old_d = subgame.d();
    subgame.compress_priorities();
    info( "Priority compression removed %d of %d priorities.",
          old_d - subgame.d(), old_d );

    // Solve the subgame
    info("Solving subgame...", (int)num_vertices);
    std::auto_ptr<LiftingStrategy> spm_lift_strat(
        LiftingStrategy::create(subgame, lift_strat_.c_str()) );
    assert(spm_lift_strat.get() != NULL);

    // Solve subgame, merging statistics back into our statistics object:
    // FIXME: now the subgame can't be aborted :/
    // FIXME: can't reuse old lifting strategy because it hasn't been
    //        initialized for this game;need to rethink how lifting
    //        strategies work.
    // (see also SmallProgressMeasures for similar problems)
    ParityGame::Strategy substrat;
    if (stats_ == NULL)
    {
        // Solve subgame without collecting statistics:
        SmallProgressMeasures subsolver(subgame, *spm_lift_strat, NULL);
        ParityGame::Strategy res = subsolver.solve();
        substrat.swap(res);
        mem += subsolver.memory_use();
    }
    else
    {
        // Solve subgame and merge statistics back into our statistics object:
        LiftingStatistics substats(subgame);
        SmallProgressMeasures subsolver(subgame, *spm_lift_strat, &substats);
        ParityGame::Strategy res = subsolver.solve();
        substrat.swap(res);
        stats_->merge(substats, vertices);
        mem += subsolver.memory_use();
    }

    if (substrat.empty())
    {
        error("Solving failed!\n");
        return 1;
    }

    // Copy strategy from subgame
    assert(substrat.size() == num_vertices + 2);  /* + 2 for 2 dummy vertices */
    for (size_t n = 0; n < num_vertices; ++n)
    {
        strategy_[vertices[n]] = substrat[n];
    }

    // Update (peak) memory use
    if (mem > memory_used_) memory_used_ = mem;

    return aborted() ? -1 : 0;
}
