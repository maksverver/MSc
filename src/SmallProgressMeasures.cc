// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "SmallProgressMeasures.h"
#include "attractor.h"
#include "SCC.h"
#include <algorithm>
#include <memory>
#include <assert.h>
#include <string.h>

LiftingStatistics::LiftingStatistics(const ParityGame &game)
    : lifts_attempted_(0), lifts_succeeded_(0)
{
    vertex_stats_.resize(game.graph().V());
}

void LiftingStatistics::record_lift(verti v, bool success)
{
    assert(v == NO_VERTEX || v < vertex_stats_.size());

    ++lifts_attempted_;
    if (v != NO_VERTEX) ++vertex_stats_[v].first;
    if (success)
    {
        ++lifts_succeeded_;
        if (v != NO_VERTEX) ++vertex_stats_[v].second;
    }
}

SmallProgressMeasures::SmallProgressMeasures(
    const ParityGame &game, LiftingStrategyFactory &lsf,
    LiftingStatistics *stats, const verti *vmap, const verti vmap_size )
        : ParityGameSolver(game), lsf_(lsf), len_(game.d()/2),
          stats_(stats), vmap_(vmap), vmap_size_(vmap_size)
{
    // Initialize SPM vector bounds
    M_ = new verti[len_];
    for (int n = 0; n < len_; ++n) M_[n] = game_.cardinality(2*n + 1) + 1;

    // Initialize SPM vector data
    size_t n = (size_t)len_*game.graph().V();
    spm_ = new verti[n];
    std::fill_n(spm_, n, 0);
}

SmallProgressMeasures::~SmallProgressMeasures()
{
    delete[] spm_;
    delete[] M_;
}

inline verti SmallProgressMeasures::get_ext_succ(verti v, bool take_max)
{
    const verti *it  = game_.graph().succ_begin(v),
                *end = game_.graph().succ_end(v);

    assert(it != end);  /* assume we have at least one successor */

    int N = len(v);
    verti res = *it++;
    for ( ; it != end; ++it)
    {
        int d = vector_cmp(*it, res, N);
        if (take_max ? d > 0 : d < 0) res = *it;
    }
    return res;
}

verti SmallProgressMeasures::get_min_succ(verti v)
{
    return get_ext_succ(v, false);
}

verti SmallProgressMeasures::get_max_succ(verti v)
{
    return get_ext_succ(v, true);
}

bool SmallProgressMeasures::lift(verti v)
{
    if (is_top(v)) return false;

    bool player_even = game_.player(v) == ParityGame::PLAYER_EVEN;
    bool priority_even = game_.priority(v)%2 == 0;

    /* Find relevant successor */
    verti w = player_even ? get_min_succ(v) : get_max_succ(v);

    /* Successor is larger than current node; we must lift it. */
    if (is_top(w))
    {
        set_top(v);
        return true;
    }

    /* See if lifting is required */
    int d = vector_cmp(v, w, len(v));

    bool carry;
    if (priority_even)
    {
        if (d >= 0) return false;
        carry = false;
    }
    else /* !priority_even */
    {
        if (d > 0) return false;
        carry = true;
    }

    /* Assign successor */
    for (int n = len(v) - 1; n >= 0; --n)
    {
        vec(v)[n] = vec(w)[n] + carry;
        carry = (vec(v)[n] == M_[n]);
        if (carry) vec(v)[n] = 0;
    }
    if (carry) set_top(v);

    return true;
}

ParityGame::Strategy SmallProgressMeasures::solve()
{
    // Initialize vertices won by odd to Top. This is designed to work
    // in conjunction with preprocess_game() which should have removed the
    // non-loop outgoing edges for such vertices.
    // N.B. The DecycleSolver and DeloopSolver make this obsolete, so if we
    //      always use those, this code may well be removed!
    verti top_cnt = 0;
    for (verti v = 0; v < game_.graph().V(); ++v)
    {
        if ( game_.priority(v)%2 == 1 &&
             game_.graph().outdegree(v) == 1 &&
             *game_.graph().succ_begin(v) == v )
        {
            set_top(v);
            ++top_cnt;
        }
    }
    info("Initialized %d vert%s to top.", top_cnt, top_cnt == 1 ? "ex" : "ices");

    std::auto_ptr<LiftingStrategy> ls(lsf_.create(game(), *this));
    assert(ls.get() != NULL);

    info("Computing minimal fixed point...");
    verti vertex = NO_VERTEX;
    bool lifted = false;
    while ((vertex = ls->next(vertex, lifted)) != NO_VERTEX)
    {
        lifted = lift(vertex);
        if (stats_ != NULL) stats_->record_lift(map_vertex(vertex), lifted);
        if (aborted()) return ParityGame::Strategy();
    }

    // Construct strategy for player even:
    info("Constructing partial strategy...");
    ParityGame::Strategy strategy(game_.graph().V(), NO_VERTEX);
    std::vector<verti> won_by_odd;
    for (verti v = 0; v < game_.graph().V(); ++v)
    {
        if (is_top(v))
            won_by_odd.push_back(v);
        else
        if (game_.player(v) == ParityGame::PLAYER_EVEN)
            strategy[v] = get_min_succ(v);
    }

    size_t mem = sizeof(verti)*len_*(game_.graph().V() + 1) +
                 sizeof(game_.d())*sizeof(verti) + ls->memory_use();

    if (!won_by_odd.empty())
    {
        // Make a dual subgame of the vertices won by player Odd
        ParityGame subgame;
        info("Constructing subgame of size %ld to solve for opponent...",
             (long)won_by_odd.size());
        subgame.make_subgame(game_, won_by_odd.begin(), won_by_odd.end());
        info("Making subgame dual to the main game...");
        subgame.make_dual();

        // NB: submap is declared here, because it must outlive the solver
        std::vector<verti> submap;

        // Solve the subgame with SPM:
        std::auto_ptr<SmallProgressMeasures> subsolver;
        if (stats_ == NULL)
        {
            /* Vertex map is only used for statistics: */
            subsolver.reset(new SmallProgressMeasures(subgame, lsf_));
        }
        else
        {
            if (vmap_size_ > 0)
            {
                submap = won_by_odd;
                merge_vertex_maps( submap.begin(), submap.end(),
                                   vmap_, vmap_size_ );
                subsolver.reset(new SmallProgressMeasures( subgame,
                    lsf_, stats_, &submap[0], submap.size() ));
            }
            else
            {
                subsolver.reset(new SmallProgressMeasures( subgame,
                    lsf_, stats_, &won_by_odd[0], won_by_odd.size() ));
            }
        }
        /* N.B. in the above, odd-cycle removal has been disabled, since the
                subgame is won by Even entirely, so they cannot occur. */

        ParityGame::Strategy substrat = subsolver->solve();
        if (substrat.empty()) return ParityGame::Strategy();
        info("Merging strategies...");
        merge_strategies(strategy, substrat, won_by_odd);

        // Account for memory used by submap & subsolver:
        mem += sizeof(verti)*submap.size();
        mem += subsolver->memory_use();
    }

    update_memory_use(mem);
    assert(verify_solution());

    return strategy;
}

void SmallProgressMeasures::preprocess_game(ParityGame &game)
{
    StaticGraph &graph = const_cast<StaticGraph&>(game.graph());  // HACK
    StaticGraph::edge_list obsolete_edges;

    for (verti v = 0; v < graph.V_; ++v)
    {
        if (graph.has_succ(v, v))
        {
            // Decide what to do with the edges:
            if ((int)game.priority(v)%2 == (int)game.player(v))
            {
                // Self-edge is beneficial; remove other edges
                for ( StaticGraph::const_iterator it = graph.succ_begin(v);
                      it != graph.succ_end(v); ++it )
                {
                    if (*it != v)
                    {
                        obsolete_edges.push_back(std::make_pair(v, *it));
                    }
                }
            }
            else
            if (graph.outdegree(v) > 1)
            {
                // Self-edge is detrimental; remove it
                obsolete_edges.push_back(std::make_pair(v, v));
            }
        }
    }
    graph.remove_edges(obsolete_edges);
}

#include <stdio.h>  /* debug */

void SmallProgressMeasures::debug_print()
{
    printf("M =");
    for (int p = 0; p < game_.d(); ++p)
    {
        printf(" %d", (p%2 == 0) ? 0 : M_[p/2]);
    }
    printf("\n");

    for (verti v = 0; v < game_.graph().V(); ++v)
    {
        printf ( "%6d %c p=%d:", (int)v,
                 game_.player(v) == ParityGame::PLAYER_EVEN ? 'E' : 'O',
                 (int)game_.priority(v) );
        if (is_top(v))
        {
            printf(" T");
        }
        else
        {
            for (int p = 0; p < game_.d(); ++p)
            {
                printf(" %d", p%2 == 0 ? 0 : vec(v)[p/2]);
            }
        }
        printf("\n");
    }

    printf("Verification %s\n", verify_solution() ? "succeeded." : "failed!");
}

bool SmallProgressMeasures::verify_solution()
{
    const StaticGraph &graph = game_.graph();

    for (verti v = 0; v < graph.V(); ++v)
    {
        if (!is_top(v))
        {
            for (int p = 0; p < game_.d(); ++p)
            {
                if (p%2 == 0) continue; /* no even components stored */

                /* Ensure vector values satisfy bounds */
                if (vec(v)[p/2] >= M_[p/2])
                {
                    printf( "%d-th component of SPM vector for vertex %d "
                            "out of bounds!\n", p, (int)v );
                    return false;
                }

                if (p > game_.priority(v) && vec(v)[p/2] != 0)
                {
                    printf( "%d-th component of SPM vector for vertex %d "
                            "should be zero!\n", p/2, (int)v );
                    return false;
                }
            }
        }

        bool player_even = game_.player(v) == ParityGame::PLAYER_EVEN;
        bool priority_even = game_.priority(v)%2 == 0;

        bool all_ok = true, one_ok = false;
        for ( StaticGraph::const_iterator it = graph.succ_begin(v);
              it != graph.succ_end(v); ++it )
        {
            int d = vector_cmp(v, *it, len(v));
            bool ok = priority_even ? d >= 0 : (d > 0 || is_top(v));
            one_ok = one_ok || ok;
            all_ok = all_ok && ok;
        }

        if (!(player_even ? one_ok : all_ok))
        {
            printf( "order constraint not satisfied for vertex %d with "
                    "priority %d and player %s!\n", v, game_.priority(v),
                    player_even ? "even" : "odd" );
            return false;
        }
    }
    return true;
}

ParityGameSolver *SmallProgressMeasuresFactory::create(
    const ParityGame &game, const verti *vmap, verti vmap_size )
{
    return new SmallProgressMeasures(
        game, lsf_, stats_, vmap, vmap_size );
}
