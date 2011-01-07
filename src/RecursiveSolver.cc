// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "RecursiveSolver.h"
#include "attractor.h"
#include <set>
#include <assert.h>
//#include "Logger.h"  // debug

class Substrategy
{
private:
    friend class Reference;

    class Reference
    {
    public:
        Reference(Substrategy &s, verti v) : substrat_(s), v_(s.global(v)) { }

        Reference &operator=(verti w)
        {
            if (w != NO_VERTEX) w = substrat_.global(w);
            substrat_.strategy_[v_] = w;
            return *this;
        }

    private:
        Substrategy &substrat_;
        verti v_;
    };

public:
    //! Constructs a strategy for all the vertices in a global strategy.
    Substrategy(ParityGame::Strategy &strategy)
        : strategy_(strategy)
    {
    }

    //! Constructs a substrategy from an existing (sub)strategy and vertex map.
    Substrategy(const Substrategy &substrat, std::vector<verti> vmap)
        : strategy_(substrat.strategy_)
    {
        global_.resize(vmap.size());
        for (size_t i = 0; i < global_.size(); ++i)
        {
            global_[i] = substrat.global(vmap[i]);
        }
    }

    //! Swaps this substrategy ovbect with another.
    void swap(Substrategy &other)
    {
        strategy_.swap(other.strategy_);
        global_.swap(other.global_);
    }

    //! Returns a write-only reference to the strategy for vertex `v'.
    Reference operator[](verti v)
    {
        return Reference(*this, v);
    }

    //! Returns the winner for vertex `v' assuming it is controlled by `p'.
    ParityGame::Player winner(verti v, ParityGame::Player p)
    {
        if (strategy_[global_[v]] == NO_VERTEX) p = ParityGame::Player(1 - p);
        return p;
    }

    //! Maps local to global vertex index
    inline verti global(verti v) const
    {
        return global_.empty() ? v : global_[v];
    }

private:
    //! Reference to the global strategy
    ParityGame::Strategy &strategy_;

    //! Mapping from local to global vertex indices, or empty for identity.
    std::vector<verti> global_;
};

/*! Returns the complement of a vertex set; i.e. an ordered list of all vertex
    indices under V, from which the contents of `vertices' have been removed. */
static std::vector<verti> get_complement( verti V,
                                          const std::set<verti> &vertices )
{
    std::vector<verti> res;
    res.reserve(V - vertices.size());
    std::set<verti>::const_iterator it = vertices.begin();
    for (verti v = 0; v < V; ++v)
    {
        if (it == vertices.end() || v < *it)
        {
            res.push_back(v);
        }
        else
        {
            assert(*it == v);
            ++it;
        }
    }
    assert(it == vertices.end());
    return res;
}

RecursiveSolver::RecursiveSolver(const ParityGame &game)
    : ParityGameSolver(game)
{
}

RecursiveSolver::~RecursiveSolver()
{
}

ParityGame::Strategy RecursiveSolver::solve()
{
    ParityGame game;
    game.assign(game_);
    game.compress_priorities(0, false);
    ParityGame::Strategy strategy(game.graph().V(), NO_VERTEX);
    Substrategy substrat(strategy);
    if (!solve(game, substrat)) strategy.clear();
    return strategy;
}

// Note: assumes priorities are fully compressed in `game', so that all
//       priorities between 0 and `d' are used, or the game is empty.
bool RecursiveSolver::solve(ParityGame &game, Substrategy &strat)
{
    if (aborted()) return false;

    while (!game.empty())
    {
        const StaticGraph &graph = game.graph();
        const verti V = graph.V();

        std::vector<verti> unsolved;

        // Compute attractor set of minimum priority vertices:
        {
            std::set<verti> min_prio_attr;
            for (verti v = 0; v < V; ++v)
            {
                if (game.priority(v) == 0) min_prio_attr.insert(v);
            }
            //Logger::info("|min_prio|=%d", (int)min_prio_attr.size());
            assert(!min_prio_attr.empty());
            make_attractor_set(game, ParityGame::PLAYER_EVEN, min_prio_attr, strat);
            //Logger::info("|min_prio_attr|=%d", (int)min_prio_attr.size());
            if (min_prio_attr.size() == V) break;
            get_complement(V, min_prio_attr).swap(unsolved);
        }

        // Solve vertices not in the minimum priority attractor set:
        {
            ParityGame subgame;
            subgame.make_subgame(game, unsolved.begin(), unsolved.end());
            assert(subgame.cardinality(0) == 0);
            subgame.compress_priorities(0, false);
            Substrategy substrat(strat, unsolved);
            if (!solve(subgame, substrat)) return false;

            // Compute attractor set of all vertices won by the opponent:
            std::set<verti> lost_attr;
            for (verti v = 0; v < (verti)unsolved.size(); ++v)
            {
                if (substrat.winner(v, game.player(unsolved[v]))
                        == ParityGame::PLAYER_ODD)
                {
                    lost_attr.insert(unsolved[v]);
                }
            }
            if (lost_attr.empty()) break;
            make_attractor_set(game, ParityGame::PLAYER_ODD, lost_attr, strat);
            //Logger::info("|lost|=%d", (int)lost_attr.size());
            //Logger::info("|lost_attr|=%d", (int)lost_attr.size());
            get_complement(V, lost_attr).swap(unsolved);
        }

        // Repeat with subgame of which vertices won by odd have been removed:
        {
            ParityGame subgame;
            subgame.make_subgame(game, unsolved.begin(), unsolved.end());
            subgame.compress_priorities(0, false);
            Substrategy substrat(strat, unsolved);
            strat.swap(substrat);
            game.swap(subgame);
        }
    }

    // If we get here, then the opponent's winning set was empty; the strategy
    // for most vertices has already been initialized, except for those with
    // minimum priority. Since the whole game is won by the current player, it
    // suffices to pick an arbitrary successor for these vertices:
    const StaticGraph &graph = game.graph();
    const verti V = graph.V();
    for (verti v = 0; v < V; ++v)
    {
        if (game.priority(v) == 0)
        {
            if (game.player(v) == ParityGame::PLAYER_EVEN)
            {
                strat[v] = *graph.succ_begin(v);
            }
            else
            {
                strat[v] = NO_VERTEX;
            }
        }
    }
    return true;
}

ParityGameSolver *RecursiveSolverFactory::create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size )
{
    (void)vertex_map;       // unused
    (void)vertex_map_size;  // unused

    return new RecursiveSolver(game);
}
