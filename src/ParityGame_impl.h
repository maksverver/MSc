// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Don't include this directly; include ParityGame.h instead!

#include <assert.h>
#include <iterator>

template<class ForwardIterator>
void ParityGame::make_subgame( const ParityGame &game,
                               ForwardIterator vertices_begin,
                               ForwardIterator vertices_end )
{
    assert(this != &game);

    const StaticGraph &graph = game.graph();
    const verti num_vertices = std::distance(vertices_begin, vertices_end);
    ForwardIterator it;
    verti v;

    reset(num_vertices, game.d());

    // Create a map of old->new vertex indices
    vertex_map_t vertex_map;
    for (it = vertices_begin, v = 0; v < num_vertices; ++v, ++it)
    {
        vertex_[v] = game.vertex_[*it];
        vertex_map[*it] = v;
    }

    // Create new edge list
    StaticGraph::edge_list edges;
    for (it = vertices_begin, v = 0; v < num_vertices; ++v, ++it)
    {
        for ( StaticGraph::const_iterator jt = graph.succ_begin(*it);
              jt != graph.succ_end(*it); ++jt )
        {
            vertex_map_t::const_iterator map_it = vertex_map.find(*jt);
            if (map_it != vertex_map.end())
            {
                edges.push_back(std::make_pair(v, map_it->second));
            }
        }
    }
    graph_.assign(edges, graph.edge_dir());
    recalculate_cardinalities(num_vertices);
}
