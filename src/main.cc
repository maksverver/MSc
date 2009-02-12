#include "logging.h"
#include "ParityGame.h"
#include "LinearLiftingStrategy.h"
#include "PredecessorLiftingStrategy.h"
#include "assert.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>


#define INPUT_NONE      0
#define INPUT_RANDOM    1
#define INPUT_PGSOLVER  2

static int          arg_input_format        = INPUT_NONE;
static std::string  arg_pgsolver_file       = "";
static std::string  arg_dot_file            = "";
static std::string  arg_winners_file        = "";
static int          arg_random_size         = 1000000;
static int          arg_random_seed         =       1;
static int          arg_random_out_degree   =      10;
static int          arg_random_priorities   =      20;

static void print_usage()
{
    printf(
"Options:\n"
"  --help/-h              show help\n"
"  --input/-i <format>    input format: random or pgsolver\n"
"  --size <int>           size of randomly generated graph\n"
"  --outdegree <int>      average out-degree in randomly generated graph\n"
"  --priorities <int>     number of priorities in randomly generated game\n"
"  --seed <int>           random seed\n"
"  --dot/-d <file>        write parity game in GraphViz dot format to <file>\n"
"  --pgsolver/-p <file>   write parity game in PGSolver format to <file>\n"
"  --winners/-w <file>    write compact winners specification to <file>\n"
        );
}

static void parse_args(int argc, char *argv[])
{
    static struct option long_options[] = {
        { "help",       false, NULL, 'h' },
        { "input",      true,  NULL, 'i' },
        { "size",       true,  NULL,  1  },
        { "outdegree",  true,  NULL,  2  },
        { "priorities", true,  NULL,  3  },
        { "seed",       true,  NULL,  4  },
        { "dot",        true,  NULL, 'd' },
        { "pgsolver",   true,  NULL, 'p' },
        { "winners",    true,  NULL, 'w' },
        { NULL,         false, NULL,  0  } };

    static const char *short_options = "hi:d:p:w:";

    for (;;)
    {
        int ch = getopt_long(argc, argv, short_options, long_options, NULL);
        if (ch == -1) break;

        switch (ch)
        {
        case 'h':   /* help */
            print_usage();
            exit(0);
            break;

        case 'i':   /* input format */
            {
                if (strcmp(optarg, "random") == 0)
                {
                    arg_input_format = INPUT_RANDOM;
                }
                else
                if (strcmp(optarg, "pgsolver") == 0)
                {
                    arg_input_format = INPUT_PGSOLVER;
                }
                else
                {
                    printf("Invalid input format: %s\n", optarg);
                    exit(1);
                }
            }
            break;

        case 1:     /* random graph size */
            arg_random_size = atoi(optarg);
            break;

        case 2:     /* random graph out-degree */
            arg_random_out_degree = atoi(optarg);
            break;

        case 3:     /* random game number of priorities */
            arg_random_priorities = atoi(optarg);
            break;

        case 4:     /* random seed */
            arg_random_seed = atoi(optarg);
            break;

        case 'd':   /* dot output file */
            arg_dot_file = optarg;
            break;

        case 'p':   /* PGSolver output file */
            arg_pgsolver_file = optarg;
            break;

        case 'w':   /* winners output file */
            arg_winners_file = optarg;
            break;

        case '?':
            {
                printf("Unrecognized option!\n");
                exit(1);
            }
            break;
        }
    }

    if (arg_input_format == INPUT_NONE)
    {
        printf("No input format specified!\n");
        print_usage();
        exit(0);
    }
}

/*! Write a game description in Graphviz DOT format */
static void write_dot(const ParityGame &game, std::ostream &os)
{
    const StaticGraph &graph = game.graph();
    os << "digraph {\n";
    for (verti v = 0; v < graph.V(); ++v)
    {
        bool even = game.player(v) == ParityGame::PLAYER_EVEN;
        os << v << " ["
           << "shape=" << (even ? "diamond" : "box") << ", "
           << "label=\"" << game.priority(v) << " (" << v << ")\"]\n";

        if (graph.edge_dir() & StaticGraph::EDGE_SUCCESSOR)
        {
            for ( StaticGraph::const_iterator it = graph.succ_begin(v);
                  it != graph.succ_end(v); ++it )
            {
                os << v << " -> " << *it << ";\n";
            }
        }
        else
        {
            for ( StaticGraph::const_iterator it = graph.pred_begin(v);
                  it != graph.pred_end(v); ++it )
            {
                os << *it << " -> " << v << ";\n";
            }
        }
    }
    os << "}\n";
}


/*! Write a the game description in PGSolver format. */
static void write_pgsolver(const ParityGame &game, std::ostream &os)
{
    const StaticGraph &graph = game.graph();
    os << "parity " << graph.V() - 1 << ";\n";
    for (verti v = 0; v < graph.V(); ++v)
    {
        os << v << ' ' << game.priority(v) << ' ' << game.player(v);
        StaticGraph::const_iterator it  = graph.succ_begin(v),
                                    end = graph.succ_end(v);
        assert(it != end);
        os << ' ' << *it++;
        while (it != end) os << ',' << *it++;
        os << ";\n";
    }
}

/*! Write summary of winners. For each node, a single character is printed:
    'E' or 'O', depending on whether player Even or Odd wins the parity game
    starting from this node. */    
static void write_winners(const ParityGameSolver &solver, std::ostream &os)
{
    verti V = solver.game().graph().V(), next_newline = 80;
    for (verti i = 0; i < V; ++i)
    {
        if (i == next_newline)
        {
            os << '\n';
            next_newline += 80;
        }
        os << (solver.winner(i) == ParityGame::PLAYER_EVEN ? 'E' : 'O');
    }
    os << '\n';
}

int main(int argc, char *argv[])
{
    time_initialize();

    parse_args(argc, argv);
    ParityGame game;

    switch (arg_input_format)
    {
    case INPUT_RANDOM:
        {
            info( "Generating random parity game with %d vertices, "
                  "out-degree %d, and %d priorities...", arg_random_size,
                  arg_random_out_degree, arg_random_priorities );
            srand(arg_random_seed);

            game.make_random(
                arg_random_size, arg_random_out_degree,
                StaticGraph::EDGE_BIDIRECTIONAL, arg_random_priorities );
        }
        break;

    case INPUT_PGSOLVER:
        {
            info("Reading PGSolver input...");
            game.read_pgsolver(std::cin, StaticGraph::EDGE_BIDIRECTIONAL);
        }
        break;

    default:
        assert(0);
    }

    info("Initializing data structures...");
    PredecessorLiftingStrategy strategy(game);
    LiftingStatistics stats(game);
    SmallProgressMeasures spm(game, strategy, &stats);
    info("Preprocessing graph...");
    spm.preprocess_graph();
    info("Starting solve...");
    spm.solve();

    info("Verifying solution...");
    /* spm.debug_print(); */
    if (!spm.verify_solution()) error("Verification failed!");

    info("Total lift attempts:     %12lld", stats.lifts_attempted());
    info("Succesful lift attempts: %12lld", stats.lifts_succeeded());

    if (!arg_pgsolver_file.empty())
    {
        if (arg_pgsolver_file == "-")
        {
            write_pgsolver(game, std::cout);
        }
        else
        {
            info( "Writing PGSolver game description to file %s...",
                  arg_pgsolver_file.c_str() );
            std::ofstream ofs(arg_pgsolver_file.c_str());
            write_pgsolver(game, ofs);
            if (!ofs) error("Writing failed!");
        }
    }

    if (!arg_dot_file.empty())
    {
        if (arg_dot_file == "-")
        {
            write_dot(game, std::cout);
        }
        else
        {
            info("Writing GraphViz dot game description to file %s...",
                arg_dot_file.c_str());
            std::ofstream ofs(arg_dot_file.c_str());
            write_dot(game, ofs);
            if (!ofs) error("Writing failed!");
        }
    }

    /* Print output */
    if (!arg_winners_file.empty())
    {
        if (arg_winners_file == "-")
        {
            write_winners(spm, std::cout);
        }
        else
        {
            info("Writing winners to file %s...", arg_winners_file.c_str());
            std::ofstream ofs(arg_winners_file.c_str());
            write_winners(spm, ofs);
            if (!ofs) error("Writing failed!");
        }
    }

    info("Exiting.");
}
