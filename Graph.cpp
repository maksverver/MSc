#include "Graph.h"
#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <utility>
#include <vector>

StaticGraph::StaticGraph()
    : successors_(NULL), predecessors_(NULL),
      successor_index_(NULL), predecessor_index_(NULL)
{
    reset(0, 0, EDGE_SUCCESSOR);
}

StaticGraph::~StaticGraph()
{
    delete[] successors_;
    delete[] predecessors_;
    delete[] successor_index_;
    delete[] predecessor_index_;
}

void StaticGraph::reset(verti V, edgei E, EdgeDirection edge_dir)
{
    V_ = V;
    E_ = E_;
    edge_dir_ = edge_dir;

    delete[] successors_;
    delete[] predecessors_;
    delete[] successor_index_;
    delete[] predecessor_index_;

    if ((edge_dir & EDGE_SUCCESSOR))
    {
        successors_      = new verti[E];
        successor_index_ = new edgei[V + 1];
        for (verti v = 0; v <= V; ++v) successor_index_[v] = 0;
    }
    else
    {
        successors_      = NULL;
        successor_index_ = NULL;
    }

    if ((edge_dir_ & EDGE_PREDECESSOR))
    {
        predecessors_      = new verti[E];
        predecessor_index_ = new edgei[V + 1];
        for (verti v = 0; v <= V; ++v) predecessor_index_[v] = 0;
    }
    else
    {
        predecessors_      = NULL;
        predecessor_index_ = NULL;
    }
}

static bool edge_cmp_forward ( const std::pair<verti, verti> &a,
                               const std::pair<verti, verti> &b )
{
    return a.first < b.first || (a.first == b.first && a.second < b.second);
}
static bool edge_cmp_backward( const std::pair<verti, verti> &a,
                               const std::pair<verti, verti> &b )
{
    return a.second < b.second || (a.second == b.second && a.first < b.first);
}

void StaticGraph::make_random(verti V, unsigned out_deg, EdgeDirection edge_dir)
{
    /* Some assumptions on the RNG output range: */
    assert(RAND_MAX >= 2*out_deg);
    assert(RAND_MAX >= V);

    /* Create a random edge set, with at least one outgoing edge per node,
       and an average out-degree `out_deg`. */
    std::vector<std::pair<verti, verti> > edges;
    for (verti i = 0; i < V; ++i)
    {
        unsigned N = 1 + rand()%(2*out_deg - 1);
        for (unsigned n = 0; n < N; ++n)
        {
            verti j = rand()%V;
            edges.push_back(std::make_pair(i, j));
        }
    }
    edgei E = (edgei)edges.size();
    assert(E == edges.size());  /* detect integer overflow */

    /* Reallocate memory */
    reset(V, E, edge_dir);

    /* Edges are already sorted by predecessor */
    if (edge_dir_ & EDGE_SUCCESSOR)
    {
        /* Sort edges by predecessor first, successor second */
        sort(edges.begin(), edges.end(), edge_cmp_forward);

        /* Create successor index */
        edgei pos = 0;
        for (verti v = 0; v < V; ++v)
        {
            while (pos < E && edges[pos].first < v) ++pos;
            successor_index_[v] = pos;
        }
        successor_index_[V] = E;

        /* Create successor list */
        for (edgei e = 0; e < E; ++e) successors_[e] = edges[e].second;
    }

    if (edge_dir_ & EDGE_PREDECESSOR)
    {
        /* Sort edges by successor first, predecessor second */
        sort(edges.begin(), edges.end(), edge_cmp_backward);

        /* Create predecessor index */
        edgei pos = 0;
        for (verti v = 0; v < V; ++v)
        {
            while (pos < E && edges[pos].second < v) ++pos;
            predecessor_index_[v] = pos;
        }
        predecessor_index_[V] = E;

        /* Create predecessor list */
        for (edgei e = 0; e < E; ++e) predecessors_[e] = edges[e].first;
    }

}
