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
                                  ParityGameSolverFactory &pgsf )
    : ParityGameSolver(game), pgsf_(pgsf)
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

    // Compress vertex priorities
    int old_d = subgame.d();
    subgame.compress_priorities();
    info( "Priority compression removed %d of %d priorities.",
          old_d - subgame.d(), old_d );

    // Solve the subgame
    // FIXME: now the subgame can't be aborted :/
    info("Solving subgame...", (int)num_vertices);
    std::auto_ptr<ParityGameSolver> subsolver(
        pgsf_.create(subgame, vertices, num_vertices) );
    ParityGame::Strategy substrat = subsolver->solve();

    // Update (peak) memory use
    update_memory_use(subgame.memory_use() + subsolver->memory_use());

    if (substrat.empty()) return -1;  // solving failed

    // Copy strategy from subgame
    assert(substrat.size() == num_vertices + 2);  /* + 2 for 2 dummy vertices */
    for (size_t n = 0; n < num_vertices; ++n)
    {
        strategy_[vertices[n]] = substrat[n];
    }

    return aborted() ? -1 : 0;
}
