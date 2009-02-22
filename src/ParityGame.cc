#include "ParityGame.h"
#include <algorithm>
#include <stdlib.h>
#include <assert.h>

// Required for ParityGame::read_pbes()
#include <mcrl2/pbes/pbes.h>
#include <mcrl2/pbes/parity_game_generator.h>

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

void ParityGame::read_pbes( const std::string &file_path,
                            StaticGraph::EdgeDirection edge_dir )
{
    /* NOTE: this code assumes the vertices generated by parity_game_generator
             are numbered from 0 to num_vertices-1 with no gaps! */

    mcrl2::pbes_system::pbes<> pbes;
    pbes.load(file_path);  // TODO: handle exceptions raised here?

    // Generate min-priority parity game
    mcrl2::pbes_system::parity_game_generator pgg(pbes, true, true);

    // Build the edge list
    StaticGraph::edge_list edges;
    verti num_vertices = 1 + *pgg.get_initial_values().rbegin();
    for (verti v = 0; v < num_vertices; ++v)
    {
        std::set<unsigned> deps = pgg.get_dependencies(v);
        for ( std::set<unsigned>::const_iterator it = deps.begin();
              it != deps.end(); ++it )
        {
            verti w = (verti)*it;
            if (w >= num_vertices) num_vertices = w + 1;
            edges.push_back(std::make_pair(v, w));
        }
    }

    // Determine maximum prioirity
    int max_prio = 0;
    for (verti v = 0; v < num_vertices; ++v)
    {
        int prio = pgg.get_priority(v);
        if (prio > max_prio) max_prio = v;
    }

    // Assign vertex info and recount cardinalities
    vertex_ = new ParityGameVertex();
    reset(num_vertices, max_prio + 1);
    std::fill(cardinality_, cardinality_ + d_, 0);
    for (verti v = 0; v < num_vertices; ++v)
    {
        bool and_op = pgg.get_operation(v) ==
                      mcrl2::pbes_system::parity_game_generator::PGAME_AND;
        vertex_[v].player = and_op ? PLAYER_ODD : PLAYER_EVEN;
        vertex_[v].priority = pgg.get_priority(v);
        cardinality_[vertex_[v].priority] += 1;
    }

    // Assign graph
    graph_.assign(edges, edge_dir);
}

size_t ParityGame::memory_use() const
{
    size_t res = graph_.memory_use();
    res += sizeof(ParityGameVertex)*graph_.V();     // vertex info
    res += sizeof(verti)*d_;                        // priority frequencies
    return res;
}
