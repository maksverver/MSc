// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "DecycleSolver.h"
#include "attractor.h"
#include <memory>
#include <assert.h>

DecycleSolver::DecycleSolver(
    const ParityGame &game, ParityGameSolverFactory &pgsf,
    const verti *vmap, verti vmap_size )
    : ParityGameSolver(game), pgsf_(pgsf), vmap_(vmap), vmap_size_(vmap_size)
{
    pgsf_.ref();
}

DecycleSolver::~DecycleSolver()
{
    pgsf_.deref();
}

int DecycleSolver::operator()(const verti *scc, size_t scc_size)
{
    // Search for a vertex with minimum priority, with a successor in the SCC:
    for (size_t i = 0; i < scc_size; ++i) {
        verti v = scc[i], orig_v = mapping_[v];
        if (game_.priority(orig_v) == prio_) {
            // Search for an edge inside the component:
            // FIXME: complexity analysis? has_succ is not constant time!
            for (size_t j = 0; j < scc_size; ++j)
            {
                verti w = scc[j];
                if (graph_.has_succ(v, w))
                {
                    winning_.push_back(orig_v);
                    if (game_.player(orig_v) == prio_%2)
                    {
                        strategy_[orig_v] = mapping_[w];
                    }
                    return 0;
                }
            }
            assert(scc_size == 1);
        }
    }
    return 0;  // continue enumerating SCCs
}

/* Returns how much memory is currently allocated for this object only: */
size_t DecycleSolver::my_memory_use()
{
    return sizeof(*this)
        + sizeof(verti)*mapping_.capacity() +
        + graph_.memory_use()
        + sizeof(verti)*winning_.size()*2  /* estimate! */ +
        + sizeof(verti)*strategy_.capacity();
}

ParityGame::Strategy DecycleSolver::solve()
{
    if (!strategy_.empty()) return strategy_;

    info( "(DecycleSolver) Searching for winner-controlled cycles...");
    const verti V = game_.graph().V();
    strategy_.assign(V, NO_VERTEX);
    DenseSet<verti> solved(0, V);
    for (prio_ = 0; prio_ < game_.d(); ++prio_)
    {
        // Find set of unsolved vertices with priority >= prio_
        for (verti v = 0; v < V; ++v)
        {
            if ( solved.count(v) == 0 &&
                    game_.priority(v) >= prio_ &&
                    ( game_.player(v) == prio_%2 ||
                    game_.graph().outdegree(v) == 1 ) )
            {
                mapping_.push_back(v);
            }
        }

        // Construct subgraph induced by vertices found above:
        graph_.make_subgraph(
            game_.graph(), mapping_.begin(), mapping_.end() );

        // Identify vertices which are part of the winning set:
        decompose_graph(graph_, *this);

        update_memory_use(my_memory_use() + solved.memory_use());
        graph_.clear();
        mapping_.clear();

        if (!winning_.empty())
        {
            verti old_solved = (verti)solved.size();

            // Compute attractor set and associated strategy:
            for ( std::deque<verti>::const_iterator it = winning_.begin();
                    it != winning_.end(); ++it ) solved.insert(*it);
            make_attractor_set( game_, (ParityGame::Player)(prio_%2),
                                solved, winning_, &strategy_ );

            update_memory_use(my_memory_use() + solved.memory_use());
            winning_.clear();

            info( "(DecycleSolver) Identified %d vertices in %d-dominated "
                    "cycles.", (verti)solved.size() - old_solved, prio_ );
        }

        // Early out: if all vertices are solved, continuing is pointless.
        if (solved.size() == V) return strategy_;
    }

    if (solved.empty())
    {
        // Don't construct a subgame if it is identical to the input game:
        info("(DecycleSolver) No suitable cycles found! Solving...");
        std::auto_ptr<ParityGameSolver> subsolver(
            pgsf_.create(game_, vmap_, vmap_size_) );
        strategy_ = subsolver->solve();
        return strategy_;
    }

    const verti num_unsolved = V - (verti)solved.size();
    info( "(DecycleSolver) Creating subgame with %d vertices remaining...",
            num_unsolved );

    // Gather remaining unsolved vertices:
    std::vector<verti> unsolved;
    unsolved.reserve(num_unsolved);
    for (verti v = 0; v < V; ++v)
    {
        if (solved.count(v) == 0) unsolved.push_back(v);
    }
    assert(!unsolved.empty() && unsolved.size() == num_unsolved);

    ParityGame subgame;
    subgame.make_subgame(game_, unsolved.begin(), unsolved.end());

    // Construct solver:
    std::vector<verti> submap; // declared here so it survives subsolver
    std::auto_ptr<ParityGameSolver> subsolver;
    if (vmap_size_ > 0)
    {
        // Need to create merged vertex map:
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

    info( "(DecycleSolver) Solving...");
    ParityGame::Strategy substrat = subsolver->solve();
    if (!substrat.empty())
    {
        info( "(DecycleSolver) Merging strategies...");
        merge_strategies(strategy_, substrat, unsolved);
    }
    update_memory_use( my_memory_use() + solved.memory_use() +
        sizeof(verti)*unsolved.capacity() + subgame.memory_use() +
        sizeof(verti)*substrat.size() + subsolver->memory_use() );

    return strategy_;
}

ParityGameSolver *DecycleSolverFactory::create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size )
{
    return new DecycleSolver(game, pgsf_, vertex_map, vertex_map_size);
}
