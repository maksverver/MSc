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
    int max_depth, const verti *vmap, verti vmap_size )
    : ParityGameSolver(game), pgsf_(pgsf), max_depth_(max_depth),
      vmap_(vmap), vmap_size_(vmap_size)
{
    pgsf_.ref();
}

ComponentSolver::~ComponentSolver()
{
    pgsf_.deref();
}

ParityGame::Strategy ComponentSolver::solve()
{
    verti V = game_.graph().V();
    strategy_.assign(V, NO_VERTEX);
    DenseSet<verti> W0(0, V), W1(0, V);
    winning_[0] = &W0;
    winning_[1] = &W1;
    if (decompose_graph(game_.graph(), *this) != 0) strategy_.clear();
    winning_[0] = NULL;
    winning_[1] = NULL;
    ParityGame::Strategy result;
    result.swap(strategy_);
    return result;
}

int ComponentSolver::operator()(const verti *vertices, size_t num_vertices)
{
    if (aborted()) return -1;

    assert(num_vertices > 0);

    // Filter out solved vertices:
    std::vector<verti> unsolved;
    unsolved.reserve(num_vertices);
    for (size_t n = 0; n < num_vertices; ++n)
    {
        verti v = vertices[n];
        if (!winning_[0]->count(v) && !winning_[1]->count(v))
        {
            unsolved.push_back(vertices[n]);
        }
    }
    info("(ComponentSolver) SCC of size %ld with %ld unsolved vertices...",
         (long)num_vertices, (long)unsolved.size());
    if (unsolved.empty()) return 0;

    // Construct a subgame for unsolved vertices in this component:
    ParityGame subgame;
    subgame.make_subgame(game_, unsolved.begin(), unsolved.end(), true);

    ParityGame::Strategy substrat;
    if (max_depth_ > 0 && unsolved.size() < num_vertices)
    {
        info( "(ComponentSolver) Recursing on subgame of size %d...",
              (int)unsolved.size() );
        ComponentSolver(subgame, pgsf_, max_depth_ - 1).solve().swap(substrat);
    }
    else
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
        info( "(ComponentSolver) Solving subgame of size %d...",
              (int)unsolved.size() );
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
        subsolver->solve().swap(substrat);
    }
    if (substrat.empty()) return -1;  // solving failed

    info("(ComponentSolver) Merging strategies...");
    merge_strategies(strategy_, substrat, unsolved);

    info("(ComponentSolver) Building attractor sets for winning regions...");

    // Extract winning sets from subgame:
    std::deque<verti> todo[2];
    for (size_t n = 0; n < unsolved.size(); ++n)
    {
        ParityGame::Player pl = subgame.winner(substrat, n);
        verti v = unsolved[n];
        winning_[pl]->insert(v);
        todo[pl].push_back(v);
    }

    // Extend winning sets to attractor sets:
    for (int player = 0; player < 2; ++player)
    {
        make_attractor_set( game_, (ParityGame::Player)player,
                            *winning_[player], todo[player], strategy_ );
    }

    info("(ComponentSolver) Leaving.");
    return 0;
}

ParityGameSolver *ComponentSolverFactory::create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size )
{
    return new ComponentSolver( game, pgsf_, max_depth_, 
                                vertex_map, vertex_map_size );
}
