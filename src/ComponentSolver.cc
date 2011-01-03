// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "ComponentSolver.h"
#include "attractor.h"

#include <assert.h>
#include <memory>

ComponentSolver::ComponentSolver(
    const ParityGame &game, ParityGameSolverFactory &pgsf,
    const verti *vmap, verti vmap_size )
    : ParityGameSolver(game), pgsf_(pgsf), vmap_(vmap), vmap_size_(vmap_size)
{
    pgsf_.ref();
}

ComponentSolver::~ComponentSolver()
{
    pgsf_.deref();
}

ParityGame::Strategy ComponentSolver::solve()
{
    if (strategy_.empty())
    {
        strategy_.assign(game_.graph().V(), NO_VERTEX);
        solved_.assign(game_.graph().V(), false);
        if (decompose_graph(game_.graph(), *this) != 0) strategy_.clear();
        solved_.clear();
    }
    return strategy_;
}

int ComponentSolver::operator()(const verti *vertices, size_t num_vertices)
{
    if (aborted()) return -1;

    assert(num_vertices > 0);

    // Filter out solved vertices:
    info("(ComponentSolver) Filtering %d vertices...", (int)num_vertices);
    std::vector<verti> unsolved;
    unsolved.reserve(num_vertices);
    for (size_t n = 0; n < num_vertices; ++n)
    {
        if (!solved_[vertices[n]]) unsolved.push_back(vertices[n]);
    }
    if (unsolved.empty()) return 0;

    // Construct a subgame for unsolved vertices in this component:
    info( "(ComponentSolver) Constructing subgame with %d vertices...",
          (int)unsolved.size() );
    ParityGame subgame;
    subgame.make_subgame(game_, unsolved.begin(), unsolved.end());

    /* N.B. if unsolved.size() < num_vertices then we run the SCC decomposition
       algorithm again (because removing vertices in attractor sets of winning
       regions may cause components to fall apart into distinct SCCs).

       In some (degenerate?) cases this could lead to a lot of work spent doing
       SCC decomposition for relatively little actual solving. To prevent this,
       change the expression in the if-statement below to `true', and then the
       components solved are the strongly-connectected components identified in
       the initial graph (and not new components created after removing vertices
       in attractor sets of winning regions). */

    if (unsolved.size() == num_vertices)
    {
        // Compress vertex priorities
        {
            int old_d = subgame.d();
            subgame.compress_priorities();
            int new_d = subgame.d();
            if (old_d != new_d)
            {
                info( "(ComponentSolver) Priority compression removed "
                      "%d of %d priorities.", old_d - new_d, old_d );
            }
        }

        // Solve the subgame
        info("(ComponentSolver) Solving subgame...");
        std::vector<verti> submap;  // declared here so it survives subsolver
        std::auto_ptr<ParityGameSolver> subsolver;
        if (vmap_size_ > 0)
        {
            submap = unsolved;
            merge_vertex_maps(submap.begin(), submap.end(), vmap_, vmap_size_);
            subsolver.reset(
                pgsf_.create(subgame, &submap[0], submap.size()) );
        }
        else
        {
            subsolver.reset(
                pgsf_.create(subgame, &unsolved[0], unsolved.size()) );
        }
        ParityGame::Strategy substrat = subsolver->solve();

        // Update (peak) memory use
        update_memory_use(subgame.memory_use() + subsolver->memory_use());
        if (substrat.empty()) return -1;  // solving failed
        merge_strategies(strategy_, substrat, unsolved);

        info("(ComponentSolver) Building attractor sets for winning regions...");

        // Extract winning sets from subgame:
        HASH_SET(verti) winning[2];
        for (size_t n = 0; n < unsolved.size(); ++n)
        {
            winning[subgame.winner(substrat, n)].insert(unsolved[n]);
        }

        // Extend winning sets to attractor sets:
        for (int player = 0; player < 2; ++player)
        {
            make_attractor_set( game_, (ParityGame::Player)player,
                                winning[player], &strategy_ );

            // Mark vertices in winning + attractor set as solved:
            for (HASH_SET(verti)::const_iterator it =
                    winning[player].begin(); it != winning[player].end(); ++it)
            {
                solved_[*it] = true;
            }
        }
    }
    else
    {
        info("(ComponentSolver) Identifying subcomponents...");
        ComponentSolver subsolver(subgame, pgsf_);
        ParityGame::Strategy substrat = subsolver.solve();
        update_memory_use(subgame.memory_use() + subsolver.memory_use());
        if (substrat.empty()) return -1;
        merge_strategies(strategy_, substrat, unsolved);
    }

    return 0;
}

ParityGameSolver *ComponentSolverFactory::create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size )
{
    return new ComponentSolver(game, pgsf_, vertex_map, vertex_map_size);
}
