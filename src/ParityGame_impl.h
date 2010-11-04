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

    const verti num_vertices = std::distance(vertices_begin, vertices_end);
    ForwardIterator it;
    verti v;

    reset(num_vertices, game.d());
    for (it = vertices_begin, v = 0; v < num_vertices; ++v, ++it)
    {
        vertex_[v] = game.vertex_[*it];
    }
    graph_.make_subgraph(game.graph_, vertices_begin, vertices_end);
    recalculate_cardinalities(num_vertices);
}
