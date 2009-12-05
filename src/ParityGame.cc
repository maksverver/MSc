// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "ParityGame.h"
#include "SCC.h"
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <assert.h>

ParityGame::ParityGame()
    : d_(0), vertex_(NULL), cardinality_(NULL)
{
}

ParityGame::~ParityGame()
{
    delete[] vertex_;
    delete[] cardinality_;
}

void ParityGame::reset(verti V, int d)
{
    delete[] vertex_;
    delete[] cardinality_;

    d_ = d;
    vertex_ = new ParityGameVertex[V];
    cardinality_ = new verti[d_];
}

void ParityGame::recalculate_cardinalities(verti num_vertices)
{
    std::fill(cardinality_, cardinality_ + d_, 0);
    for (verti v = 0; v < num_vertices; ++v)
    {
        cardinality_[vertex_[v].priority] += 1;
    }
}

void ParityGame::make_random( verti V, unsigned out_deg,
                              StaticGraph::EdgeDirection edge_dir, int d )
{
    graph_.make_random(V, out_deg, edge_dir);
    reset(V, d);
    for (verti v = 0; v < V; ++v)
    {
        vertex_[v].player   = (rand()%2 == 0) ? PLAYER_EVEN : PLAYER_ODD;
        vertex_[v].priority = rand()%d;
    }
    recalculate_cardinalities(V);
}

void ParityGame::make_subgame( const ParityGame &game,
                               const verti *vertices, verti num_vertices,
                               const Strategy &strategy )
{
    const StaticGraph &graph = game.graph();
    reset(num_vertices + 2, game.d());

    // Create dummy vertex won by even
    const verti v_even = num_vertices + 0;
    vertex_[v_even].player   = PLAYER_EVEN;
    vertex_[v_even].priority = 0;

    // Create dummy vertex won by odd
    const verti v_odd  = num_vertices + 1;
    vertex_[v_odd].player   = PLAYER_ODD;
    vertex_[v_odd].priority = 1;

    // Create a map of old->new vertex indices
    // TODO: replace this with a hash map for better performance?
    std::map<verti, verti> vertex_map;
    for (verti n = 0; n < num_vertices; ++n)
    {
        vertex_[n] = game.vertex_[vertices[n]];
        vertex_map[vertices[n]] = n;
    }

    // Create new edge list
    StaticGraph::edge_list edges;
    for (verti v = 0; v < num_vertices; ++v)
    {
        for ( StaticGraph::const_iterator it = graph.succ_begin(vertices[v]);
              it != graph.succ_end(vertices[v]); ++it )
        {
            verti w;
            std::map<verti, verti>::const_iterator map_it = vertex_map.find(*it);
            if (map_it != vertex_map.end())
            {
                w = map_it->second;
            }
            else
            {
                w = (winner(strategy, *it) == PLAYER_EVEN) ? v_even : v_odd;
            }
            edges.push_back(std::make_pair(v, w));
        }
    }
    edges.push_back(std::make_pair(v_even, v_even));
    edges.push_back(std::make_pair(v_odd,  v_odd));
    graph_.assign(edges, graph.edge_dir());
    recalculate_cardinalities(num_vertices + 2);
}

void ParityGame::make_subgame( const ParityGame &game,
                               const verti *vertices, verti num_vertices )
{
    const StaticGraph &graph = game.graph();
    reset(num_vertices, game.d());

    // Create a map of old->new vertex indices
    // TODO: replace this with a hash map for better performance?
    std::map<verti, verti> vertex_map;
    for (verti n = 0; n < num_vertices; ++n)
    {
        vertex_[n] = game.vertex_[vertices[n]];
        vertex_map[vertices[n]] = n;
    }

    // Create new edge list
    StaticGraph::edge_list edges;
    for (verti v = 0; v < num_vertices; ++v)
    {
        for ( StaticGraph::const_iterator it = graph.succ_begin(vertices[v]);
              it != graph.succ_end(vertices[v]); ++it )
        {
            std::map<verti, verti>::const_iterator map_it = vertex_map.find(*it);
            if (map_it != vertex_map.end())
            {
                edges.push_back(std::make_pair(v, map_it->second));
            }
        }
    }
    graph_.assign(edges, graph.edge_dir());
    recalculate_cardinalities(num_vertices);
}

void ParityGame::make_dual()
{
    // For each vertex, invert player and increase priority by one
    for (verti v = 0; v < graph_.V(); ++v)
    {
        vertex_[v].player   = (Player)vertex_[v].player ^ 1;
        vertex_[v].priority = vertex_[v].priority + 1;
    }

    // Update priority counts (move each on space to the right)
    verti *new_cardinality = new verti[d_ + 1];
    new_cardinality[0] = 0;
    std::copy(cardinality_, cardinality_ + d_, new_cardinality + 1);
    delete[] cardinality_;
    cardinality_ = new_cardinality;
    d_ = d_ + 1;

    // Try to compress priorities
    compress_priorities();
}

void ParityGame::shuffle(const std::vector<verti> &perm)
{
    // N.B. maximum priority and priorities cardinalities remain unchanged.

    /* NOTE: shuffling could probably be done more efficiently (in-place?)
             if performance becomes an issue. */

    // Create new edge list
    StaticGraph::edge_list edges;
    for (verti v = 0; v < graph_.V(); ++v)
    {
        for ( StaticGraph::const_iterator it = graph_.succ_begin(v);
              it != graph_.succ_end(v); ++it )
        {
            verti w = *it;
            edges.push_back(std::make_pair(perm[v], perm[w]));
        }
    }
    graph_.assign(edges, graph_.edge_dir());

    // Create new vertex info
    ParityGameVertex *new_vertex = new ParityGameVertex[graph_.V()];
    for (verti v = 0; v < graph_.V(); ++v) new_vertex[perm[v]] = vertex_[v];
    delete vertex_;
    vertex_ = new_vertex;
}

void ParityGame::compress_priorities()
{
    // Quickly check if we have anything to compress first:
    if (std::find(cardinality_ + 1, cardinality_ + d_, 0) == cardinality_ + d_)
    {
        return;
    }

    // Find out how to map old priorities to new priorities
    std::vector<int> prio_map(d_, -1);
    int last_prio = 0;
    prio_map[0] = last_prio;
    for (int p = 1; p < d_; ++p)
    {
        if (cardinality_[p] == 0) continue;  // remove priority p
        if (last_prio%2 != p%2) ++last_prio;
        prio_map[p] = last_prio;
    }

    // Remap priorities of all vertices
    for (verti v = 0; v < graph_.V(); ++v)
    {
        assert(prio_map[vertex_[v].priority] >= 0);
        vertex_[v].priority = prio_map[vertex_[v].priority];
    }

    // Update priority limit and cardinality counts
    int new_d = last_prio + 1;
    assert(new_d < d_);
    verti *new_cardinality = new verti[new_d];
    std::fill(new_cardinality, new_cardinality + new_d, 0);
    for (int p = 0; p < d_; ++p)
    {
        if (prio_map[p] >= 0)
        {
            new_cardinality[prio_map[p]] += cardinality_[p];
        }
    }
    delete[] cardinality_;
    cardinality_ = new_cardinality;
    d_ = new_d;
}

size_t ParityGame::memory_use() const
{
    size_t res = graph_.memory_use();
    res += sizeof(ParityGameVertex)*graph_.V();     // vertex info
    res += sizeof(verti)*d_;                        // priority frequencies
    return res;
}

ParityGame::Player ParityGame::winner(const Strategy &s, verti v) const
{
    /* A vertex is won by its player iff the player has a strategy for it: */
    return (s[v] != NO_VERTEX) ? player(v) : ParityGame::Player(1 - player(v));
}

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
    assert(s.size() != graph_.V());

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
           VerifySCC::operator(). */

        // Create a subgraph storing successors only:
        StaticGraph subgraph;
        subgraph.assign(edges, StaticGraph::EDGE_SUCCESSOR);

        // Find a vertex with priority prio on a cycle:
        VerifySCC verifier = { *this, subgraph, prio };
        if (decompose_graph(subgraph, verifier) != 0) return false;
    }

    return false;
}
