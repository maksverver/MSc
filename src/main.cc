#include "logging.h"
#include "timing.h"
#include "SCC.h" // for testing
#include "ParityGame.h"
#include "LinearLiftingStrategy.h"
#include "PredecessorLiftingStrategy.h"

#include <aterm_init.h>

#include "assert.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>

enum InputFormat {
    INPUT_NONE = 0, INPUT_RANDOM, INPUT_PGSOLVER, INPUT_PBES
};

static InputFormat  arg_input_format        = INPUT_NONE;
static std::string  arg_pgsolver_file       = "";
static std::string  arg_dot_file            = "";
static std::string  arg_winners_file        = "";
static bool         arg_scc_decomposition   = false;
static int          arg_random_size         = 1000000;
static int          arg_random_seed         =       1;
static int          arg_random_out_degree   =      10;
static int          arg_random_priorities   =      20;

/* TODO: eliminate these global variables; wrap them in a solver class
         with a SCC callback method instead. */
static ParityGame                       game;
static std::vector<ParityGame::Player>  winners;

struct MStat {
    int size, resident, share, text, lib, data, dt;
};

static bool read_mstat(MStat &stat)
{
    std::ifstream ifs("/proc/self/statm");
    return ifs >> stat.size >> stat.resident >> stat.share >> stat.text
               >> stat.lib >> stat.data >> stat.dt;
}

static double get_vmsize()
{
    MStat mstat;
    if (!read_mstat(mstat)) return -1;
    return (double)mstat.size*getpagesize()/1048576;
}

static void print_usage()
{
    printf(
"Options:\n"
"  --help/-h              show help\n"
"  --input/-i <format>    input format: random, PGSolver or PBES\n"
"  --size <int>           size of randomly generated graph\n"
"  --outdegree <int>      average out-degree in randomly generated graph\n"
"  --priorities <int>     number of priorities in randomly generated game\n"
"  --seed <int>           random seed\n"
"  --dot/-d <file>        write parity game in GraphViz dot format to <file>\n"
"  --pgsolver/-p <file>   write parity game in PGSolver format to <file>\n"
"  --winners/-w <file>    write compact winners specification to <file>\n"
"  --scc                  solve strongly connected components individually\n"
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
        { "scc",        false, NULL,  5  },
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
                if (strcasecmp(optarg, "random") == 0)
                {
                    arg_input_format = INPUT_RANDOM;
                }
                else
                if (strcasecmp(optarg, "pgsolver") == 0)
                {
                    arg_input_format = INPUT_PGSOLVER;
                }
                else
                if (strcasecmp(optarg, "pbes") == 0)
                {
                    arg_input_format = INPUT_PBES;
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

        case 5:     /* decompose into strongly connected components */
            arg_scc_decomposition = true;
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
static void write_winners( std::ostream &os,
                           const std::vector<ParityGame::Player> winners )
{
    size_t next_newline = 80;
    for (size_t n = 0; n < winners.size(); ++n)
    {
        if (n == next_newline)
        {
            os << '\n';
            next_newline += 80;
        }
        os << ( (winners[n] == ParityGame::PLAYER_EVEN) ? 'E' :
                (winners[n] == ParityGame::PLAYER_ODD)  ? 'O' : '?' );
    }
    os << '\n';
}

int callback(const verti *vertices, size_t num_vertices)
{
    std::cout << "Component found:";
    for (size_t n = 0; n < num_vertices; ++n) std::cout << ' ' << vertices[n];
    std::cout << std::endl;
    return 0;
}

bool read_input(ParityGame &game)
{
    switch (arg_input_format)
    {
    case INPUT_RANDOM:
        info( "Generating random parity game with %d vertices, "
                "out-degree %d, and %d priorities...", arg_random_size,
                arg_random_out_degree, arg_random_priorities );
        srand(arg_random_seed);

        game.make_random(
            arg_random_size, arg_random_out_degree,
            StaticGraph::EDGE_BIDIRECTIONAL, arg_random_priorities );

        return true;

    case INPUT_PGSOLVER:
        info("Reading PGSolver input...");
        game.read_pgsolver(std::cin, StaticGraph::EDGE_BIDIRECTIONAL);
        return true;

    case INPUT_PBES:
        info("Generating parity game from PBES input....");
        game.read_pbes("", StaticGraph::EDGE_BIDIRECTIONAL);
        return true;

    case INPUT_NONE:
        return false;
    }

    return false;
}

void write_output( const ParityGame &game,
                   const std::vector<ParityGame::Player> winners )
{
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
            if (!std::cout) error("Writing failed!");
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
            write_winners(std::cout, winners);
            if (!std::cout) error("Writing failed!");
        }
        else
        {
            info("Writing winners to file %s...", arg_winners_file.c_str());
            std::ofstream ofs(arg_winners_file.c_str());
            write_winners(ofs, winners);
            if (!ofs) error("Writing failed!");
        }
    }
}

int scc_callback(const verti *vertices, size_t num_vertices)
{
    info("Constructing subgame with %d vertices...", (int)num_vertices);

    // Construct a subgame
    ParityGame subgame;
    subgame.make_subgame(game, vertices, num_vertices, &winners[0]);

    // Compress vertex priorities
    int old_d = subgame.d();
    subgame.compress_priorities();
    info( "Priority compression removed %d of %d priorities.",
          old_d - subgame.d(), old_d );

    // Solve the subgame
    info("Solving subgame...", (int)num_vertices);
    PredecessorLiftingStrategy strategy(subgame);
    SmallProgressMeasures spm(subgame, strategy, NULL);
    if (!spm.solve()) fatal("Solving failed!\n");

    // Copy winners from subgame
    for (size_t n = 0; n < num_vertices; ++n)
    {
        winners[vertices[n]] = spm.winner(n);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    time_initialize();

    MCRL2_ATERMPP_INIT(argc, argv);

    parse_args(argc, argv);

    if (!read_input(game))
    {
        fatal("Couldn't parse parity game from input!\n");
    }

    // Prepare winners vector
    winners.clear();
    winners.insert(winners.end(), game.graph().V(), ParityGame::PLAYER_NONE);

    // Solve game by decomposing into SCC's
    double solve_time = time_used();
    info("Starting solve...");
    if (arg_scc_decomposition)
    {
        /* Decompose graph into SCC; the SCC callback creates subgames from
           components and solves these. */
        decompose_graph(game.graph(), scc_callback);
    }
    else
    {
        PredecessorLiftingStrategy strategy(game);
        SmallProgressMeasures spm(game, strategy, NULL);
        if (!spm.solve()) fatal("Solving failed!\n");
        for (size_t n = 0; n < winners.size(); ++n)
        {
            winners[n] = spm.winner(n);
        }
    }
    solve_time = time_used() - solve_time;

    // Print some statistics
    //size_t total_memory_use = game.memory_use() + spm.memory_use();
    info("Time used to solve:        %11.3fs", solve_time);
    //info("Memory used (measured):    %10.3fMB", get_vmsize());
    //info("Memory used (calculated):  %10.3fMB", total_memory_use/1048576.0);
    //info("    used by parity game:   %10.3fMB", game.memory_use()/1048576.0);
    //info("        used by graph:     %10.3fMB", game.graph().memory_use()/1048576.0);
    //info("    used by solver:        %10.3fMB", spm.memory_use()/1048576.0);
    //info("Total lift attempts:       %12lld", stats.lifts_attempted());
    //info("Succesful lift attempts:   %12lld", stats.lifts_succeeded());
    //info("Minimum lifts required:    %12lld", 0LL);  // TODO

    /* spm.debug_print(); */

    write_output(game, winners);

    info("Exiting.");
}
