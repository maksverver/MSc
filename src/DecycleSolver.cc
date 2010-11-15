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
                    winning_.insert(orig_v);
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

ParityGame::Strategy DecycleSolver::solve()
{
    if (strategy_.empty())
    {
        const verti V = game_.graph().V();
        strategy_.assign(V, NO_VERTEX);
        std::vector<bool> solved(V, false);
        verti num_solved = 0;

        info( "(DecycleSolver) Searching for cycles for %d priorities...",
               game_.d() );
        for (prio_ = 0; prio_ < game_.d(); ++prio_)
        {
            // Find set of unsolved vertices with priority >= prio_
            for (verti v = 0; v < V; ++v)
            {
                if ( !solved[v] && game_.priority(v) >= prio_ &&
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
            graph_.clear();
            mapping_.clear();

            if (!winning_.empty())
            {
                // Compute attractor set and associated strategy:
                // FIXME: this may change the strategy for already-solved
                //        vertices! This is not wrong, but it does more work
                //        than strictly necessary... it would be better if
                //        make_attractor_set() could use solved[]
                make_attractor_set( game_, (ParityGame::Player)(prio_%2),
                                    winning_, &strategy_ );

                for ( HASH_SET(verti)::const_iterator it = winning_.begin();
                    it != winning_.end(); ++it )
                {
                    verti v = *it;
                    if (!solved[v]) ++num_solved;
                    solved[v] = true;
                }

                info( "(DecycleSolver) Identified %ld vertices in "
                      "%d-dominated cycles.", (long)winning_.size(), prio_ );

                winning_.clear();
            }

            // Early out: if all vertices are solved, continuing is pointless.
            if (num_solved == V) return strategy_;
        }

        if (num_solved == 0)
        {
            // Don't construct a subgame if it is identical to the input game:
            info("(DecycleSolver) No suitable cycles found! Solving...");
            std::auto_ptr<ParityGameSolver> subsolver(
                pgsf_.create(game_, vmap_, vmap_size_) );
            strategy_ = subsolver->solve();
            return strategy_;
        }

        info( "(DecycleSolver) Creating subgame with %d vertices remaining...",
              V - num_solved );

        // Gather remaining unsolved vertices:
        std::vector<verti> unsolved;
        unsolved.reserve(V - num_solved);
        for ( verti v = 0; v < V; ++v )
        {
            if (!solved[v]) unsolved.push_back(v);
        }
        assert(unsolved.size() == V - num_solved);

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
    }
    return strategy_;
}

ParityGameSolver *DecycleSolverFactory::create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size )
{
    return new DecycleSolver(game, pgsf_, vertex_map, vertex_map_size);
}

#if 0
/*! Nested class used to implement solve_odd_cycles(): */
class OddCycleSolver
{
public:
    OddCycleSolver(SmallProgressMeasures &spm) : spm_(spm) { };

    /*! Run the preprocessing algorithm per se: */
    void run();

    /*! Callback function for strongly-connected components: */
    int operator()(const verti *scc, size_t scc_size);

protected:
    int prio_;                    /*! current selected priority */
    std::vector<verti> mapping_;  /*! current priority induced vertex set */
    StaticGraph graph_;           /*! current priority induced subgraph */
    HASH_SET(verti) winning_;     /*! current winning vertices for Odd */
    SmallProgressMeasures &spm_;  /*! reference to solver instance */
};

int SmallProgressMeasures::OddCycleSolver::operator()(
        const verti *scc, size_t scc_size )
{
    const ParityGame &game = spm_.game();

    // Search for a vertex with minimum priority, with a successor in the SCC:
    for (size_t i = 0; i < scc_size; ++i) {
        verti v = scc[i];
        if (game.priority(mapping_[v]) == prio_) {
            if (scc_size > 1 || graph_.has_succ(v, v)) {
                // Found one!
                winning_.insert(mapping_[v]);
                break;
            }
        }
    }

    return 0;  // continue enumerating SCCs
}

void SmallProgressMeasures::OddCycleSolver::run()
{
    const ParityGame &game = spm_.game();

    // backward seems to be faster in practice, though forward is correct too:
    for (prio_ = (game.d() & ~1) - 1; prio_ >= 1; prio_ -= 2)
    {
        // Find set of non-top vertices with priority >= prio_
        for (verti v = 0; v < game.graph().V(); ++v)
        {
            if ( !spm_.is_top(v) && game.priority(v) >= prio_ &&
                 ( game.player(v) == ParityGame::PLAYER_ODD ||
                   game.graph().outdegree(v) == 1 ) )
            {
                mapping_.push_back(v);
            }
        }

        // Construct induced subgraph:
        graph_.make_subgraph(game.graph(), mapping_.begin(), mapping_.end());

        // Identify vertices which are part of the winning set:
        decompose_graph(graph_, *this);
        graph_.clear();
        mapping_.clear();

        // Set all vertices in the winning attractor set to Top:
        if (!winning_.empty())
        {
            // Compute attractor set:
            make_attractor_set(game, ParityGame::PLAYER_ODD, winning_, NULL);

            info("Set %ld Odd-controlled vertices leading to %d-cycles to Top.",
                 (long)winning_.size(), prio_);
            for ( HASH_SET(verti)::const_iterator it = winning_.begin();
                it != winning_.end(); ++it ) spm_.set_top(*it);
            winning_.clear();
        }
    }
}
#endif
