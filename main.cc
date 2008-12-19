#include "ParityGame.h"
#include "LinearLiftingStrategy.h"
#include "PredecessorLiftingStrategy.h"
#include "assert.h"
#include <fstream>
#include <iostream>

/* Dump a parity game in Graphviz DOT format */
void write_dot(const ParityGame &game, const char *path)
{
    const StaticGraph &graph = game.graph();
    std::ofstream ofs(path);
    assert(ofs.good());

    ofs << "digraph {\n";
    for (verti v = 0; v < graph.V(); ++v)
    {
        bool even = game.player(v) == ParityGame::PLAYER_EVEN;
        ofs << v << " ["
            << "shape=" << (even ? "diamond" : "box") << ", "
            << "label=\"" << game.priority(v) << " (" << v << ")\"]\n";

        if (graph.edge_dir() & StaticGraph::EDGE_SUCCESSOR)
        {
            for ( StaticGraph::const_iterator it = graph.succ_begin(v);
                  it != graph.succ_end(v); ++it )
            {
                ofs << v << " -> " << *it << ";\n";
            }
        }
        else
        {
            for ( StaticGraph::const_iterator it = graph.pred_begin(v);
                  it != graph.pred_end(v); ++it )
            {
                ofs << *it << " -> " << v << ";\n";
            }
        }
    }
    ofs << "}\n";
}

int main()
{
    ParityGame game;

    printf("Generating random parity game...\n");
    /* game.make_random(10, 2, StaticGraph::EDGE_BIDIRECTIONAL, 4); */
    game.make_random(1000000, 10, StaticGraph::EDGE_BIDIRECTIONAL, 20);

    printf("Initializing data structures...\n");
    PredecessorLiftingStrategy strategy(game);
    LiftingStatistics stats(game);
    SmallProgressMeasures spm(game, strategy, &stats);
    printf("Starting solve...\n");
    spm.solve();

    /* spm.debug_print(); */

    printf("Verifying solution...\n");
    /* assert(spm.verify_solution()); */

    printf("Total lift attempts:     %12lld\n", stats.lifts_attempted());
    printf("Succesful lift attempts: %12lld\n", stats.lifts_succeeded());
    write_dot(game, "test.dot");
}
