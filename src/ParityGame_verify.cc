#include "ParityGame.h"
#include "SCC.h"
#include <assert.h>

struct VerifySCC  // used by ParityGame::verify
{
    const ParityGame    &_game;
    const StaticGraph   &_graph;
    const int           _prio;

    int operator() (const verti *scc, size_t scc_size)
    {
        // Search vertices in this SCC for a vertex with priority `prio':
        for (size_t i = 0; i < scc_size; ++i)
        {
            verti v = scc[i];
            if (_game.priority(v) == _prio)
            {
                // Cycle detected if |SCC| > 1 or v has a self-edge:
                if (scc_size > 1 || _graph.has_succ(v, v)) return 1;
            }
        }
        return 0;
    }
};

bool ParityGame::verify(const Strategy &s) const
{
    assert(s.size() == graph_.V());

    /* Make sure winning sets are consistently defined; i.e. only existent
       edges are used, and there are no transitions that cross winning sets. */
    for (verti v = 0; v < graph_.V(); ++v)
    {
        Player pl = winner(s, v);

        if (pl == player(v))  /* vertex won by owner */
        {
            // Verify owner has a strategy: (always true)
            if (s[v] == NO_VERTEX) return false;

            // Verify strategy uses existent edges:
            if (!graph_.has_succ(v, s[v])) return false;

            // Verify strategy stays within winning set:
            if (winner(s, s[v]) != pl) return false;
        }
        else  /* vertex lost by owner */
        {
            // Verify owner has no strategy: (always true)
            if (s[v] != NO_VERTEX) return false;

            // Verify owner cannot move outside this winning set:
            for (StaticGraph::const_iterator it = graph_.succ_begin(v);
                 it != graph_.succ_end(v); ++it)
            {
                if (winner(s, *it) != pl) return false;
            }
        }
    }

    // Verify absence of cycles owned by opponent in winning sets
    for (int prio = 0; prio < d_; ++prio)
    {
        /* Create set of edges incident with vertices in the winning set of
           player (1 - prio%2) consistent with strategy s and incident with
           vertices of priorities >= prio only. */
        StaticGraph::edge_list edges;
        for (verti v = 0; v < graph_.V(); ++v)
        {
            if (priority(v) >= prio && (int)winner(s, v) == (1 - prio%2))
            {
                if (s[v] != NO_VERTEX)
                {
                    if (priority(s[v]) >= prio)
                    {
                        edges.push_back(std::make_pair(v, s[v]));
                    }
                }
                else
                {
                    for (StaticGraph::const_iterator it = graph_.succ_begin(v);
                         it != graph_.succ_end(v); ++it)
                    {
                        if (priority(*it) >= prio)
                        {
                            edges.push_back(std::make_pair(v, *it));
                        }
                    }
                }
            }
        }

        /* NOTE: we should NOT compact vertices here, because then we cannot
           use their indices to determine the priority of vertices in
           VerifySCC::operator().

           Alternatively, we could compress the priority vector as well.
           We could share some code with make_subgame() to do this (except that
           we don't need the player vector, and we don't need bidirectional
           edges, which require sorting).
        */

        // Create a subgraph storing successors only:
        StaticGraph subgraph;
        subgraph.assign(edges, StaticGraph::EDGE_SUCCESSOR);

        // Find a vertex with priority prio on a cycle:
        VerifySCC verifier = { *this, subgraph, prio };
        if (decompose_graph(subgraph, verifier) != 0) return false;
    }

    return true;
}