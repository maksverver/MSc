#include "RecursiveSolver.h"
#include "attractor.h"
#include <set>
#include "assert.h"

// TODO: support aborting solver
// TODO: support reporting memory use

std::vector<verti> get_complement(verti V, const std::set<verti> &vertices)
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
    return solve(game(), 0);
}

size_t RecursiveSolver::memory_use()
{
    return 0;  // TODO
}

ParityGame::Strategy RecursiveSolver::solve(const ParityGame &game, int min_prio)
{
    assert(min_prio < game.d());

    const StaticGraph        &graph   = game.graph();
    const verti              V        = graph.V();
    const ParityGame::Player player   = (ParityGame::Player)(min_prio%2);
    const ParityGame::Player opponent = (ParityGame::Player)(1 - min_prio%2);

    if (game.d() - min_prio <= 1)
    {
        // Only one priority left; construct trivial strategy.
        ParityGame::Strategy strategy(V, NO_VERTEX);
        for (verti v = 0; v < V; ++v)
        {
            if (game.player(v) == player) strategy[v] = *graph.succ_begin(v);
        }
        return strategy;
    }

    // Compute attractor set of minimum priority vertices:
    std::set<verti> min_prio_attr;
    for (verti v = 0; v < V; ++v)
    {
        assert(game.priority(v) >= min_prio);
        if (game.priority(v) == min_prio) min_prio_attr.insert(v);
    }
    if (min_prio_attr.empty())
    {
        /* Degenerate case: no vertices with this priority exist; recurse
           directly instead of creating a subgame equal to this game: */
        return solve(game, min_prio + 1);
    }
    ParityGame::Strategy strategy(V, NO_VERTEX);
    make_attractor_set(game, player, min_prio_attr, &strategy);

    // Compute attractor set of vertices lost to the opponent:
    std::set<verti> lost_attr;
    {
        // Find unsolved vertices so far:
        std::vector<verti> unsolved = get_complement(V, min_prio_attr);

        // Create subgame with unsolved vertices:
        ParityGame subgame;
        subgame.make_subgame(game, &unsolved[0], unsolved.size());
        ParityGame::Strategy substrat = solve(subgame, min_prio + 1);

        /* Merge substrat (from formerly unsolved part) into strategy. */
        for (size_t i = 0; i < unsolved.size(); ++i)
            strategy[unsolved[i]] = substrat[i];

        // Create attractor set of all vertices won by the opponent:
        for (verti v = 0; v < (verti)unsolved.size(); ++v)
        {
            if (subgame.winner(substrat, v) == opponent)
            {
                lost_attr.insert(unsolved[v]);
            }
        }
        make_attractor_set(game, opponent, lost_attr, &strategy);
    }

    if (lost_attr.empty())  // whole game won by current player!
    {
        // Pick an arbitrary edge for minimum-priority vertices:
        for (verti v = 0; v < V; ++v)
        {
            if (game.player(v) == player && strategy[v] == NO_VERTEX)
            {
                assert(game.priority(v) == min_prio);
                strategy[v] = *graph.succ_begin(v);
            }
        }
    }
    else  // opponent wins some vertices
    {
        // Construct subgame without vertices lost to opponent:
        std::vector<verti> unsolved = get_complement(V, lost_attr);
        ParityGame subgame;
        subgame.make_subgame(game, &unsolved[0], unsolved.size());
        ParityGame::Strategy substrat = solve(subgame, min_prio);

        // Merge substrat2 into strategy:
        for (size_t i = 0; i < unsolved.size(); ++i)
            strategy[unsolved[i]] = substrat[i];
    }

    return strategy;
}
