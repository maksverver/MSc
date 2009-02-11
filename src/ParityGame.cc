#include "ParityGame.h"
#include <algorithm>
#include <stdlib.h>
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

void ParityGame::make_random( verti V, unsigned out_deg,
                              StaticGraph::EdgeDirection edge_dir, int d )
{
    graph_.make_random(V, out_deg, edge_dir);
    reset(V, d);
    std::fill(cardinality_, cardinality_ + d, 0);
    for (verti v = 0; v < V; ++v)
    {
        vertex_[v].player   = (rand()%2 == 0) ? PLAYER_EVEN : PLAYER_ODD;
        vertex_[v].priority = rand()%d;
        cardinality_[vertex_[v].priority] += 1;
    }
}

void ParityGame::read_pgsolver( std::istream &is,
                                StaticGraph::EdgeDirection edge_dir )
{
    // Read header line (if present)
    char ch = 0;
    is.get(ch);
    if (isdigit(ch))
    {
        // No header; put character back to parse later
        is.putback(ch);
    }
    else
    {
        // Skip to terminating semicolon
        while (is.get(ch) && ch != ';') ch = 0;
    }

    int max_prio = 0;
    std::vector<ParityGameVertex> vertices;
    StaticGraph::edge_list edges;

    // Read node specs
    while (is)
    {
        verti id;
        int prio, player;
        if (!(is >> id >> prio >> player)) break;

        if (prio < 0) prio = 0;
        if (prio > 127) prio = 127;
        if (player < 0) player = 0;
        if (player > 1) player = 1;

        if (prio > max_prio) max_prio = prio;
        if (id >= vertices.size()) vertices.resize(id + 1);
        vertices[id].player   = player;
        vertices[id].priority = prio;

        /* FIXME: the PGSolver file format description requires that we remove
                  existing successor edges (in case a node is defined more than
                  once). */

        // Read successors
        do {
            verti succ;
            if (!(is >> succ)) break;
            if (succ >= vertices.size()) vertices.resize(succ + 1);

            edges.push_back(std::make_pair(id, succ));

            // Skip to separator (comma) or end-of-list (semicolon)
            while (is.get(ch) && ch != ',' && ch != ';') ch = 0;

        } while (ch == ',');
    }

    // Assign vertex info and recount cardinalities
    vertex_ = new ParityGameVertex();
    reset((verti)vertices.size(), max_prio + 1);
    std::fill(cardinality_, cardinality_ + d_, 0);
    for (size_t n = 0; n < vertices.size(); ++n)
    {
        vertex_[n] = vertices[n];
        cardinality_[vertices[n].priority] += 1;
    }
    vertices.clear();

    // Assign graph
    graph_.assign(edges, edge_dir);
}
