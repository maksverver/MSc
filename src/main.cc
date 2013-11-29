// Copyright (c) 2009-2013 University of Twente
// Copyright (c) 2009-2013 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2013 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2013 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/*! \file main.cc

    The main solver tool implementation.
*/

#if defined(__unix__) || defined(__linux__)
#define POSIX
#endif

#include "ComponentSolver.h"
#include "DecycleSolver.h"
#include "DeloopSolver.h"
#include "GraphOrdering.h"
#include "Logger.h"
#include "ParityGame.h"
#include "RecursiveSolver.h"
#include "SmallProgressMeasures.h"
#include "Timer.h"
#include "shuffle.h"
#include "Decimal.h"

#include <assert.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <memory>

#ifdef POSIX
#include <unistd.h>
#include <signal.h>
#endif

#ifdef WITH_THREADS
#include <omp.h>
#include "ConcurrentRecursiveSolver.h"
#endif

#ifdef WITH_MPI
#include "MpiUtils.h"
#include "MpiSpmSolver.h"
#include "MpiRecursiveSolver.h"
#endif

#define strcasecmp compat_strcasecmp


enum InputFormat {
    INPUT_NONE = 0, INPUT_RAW, INPUT_RANDOM, INPUT_PGSOLVER, INPUT_PBES
};

static InputFormat  arg_input_format          = INPUT_NONE;
static std::string  arg_dot_file              = "";
static std::string  arg_pgsolver_file         = "";
static std::string  arg_raw_file              = "";
static std::string  arg_winners_file          = "";
static std::string  arg_strategy_file         = "";
static std::string  arg_paritysol_file        = "";
static std::string  arg_hot_vertices_file     = "";
static std::string  arg_debug_file            = "";
static std::string  arg_spm_lifting_strategy  = "";
static int          arg_spm_version           = 0;
static bool         arg_collect_stats         = false;
static bool         arg_alternate             = false;
static bool         arg_decycle               = false;
static bool         arg_deloop                = false;
static bool         arg_scc_decomposition     = false;
static bool         arg_solve_dual            = false;
static std::string  arg_reordering;
static bool         arg_priority_propagation  = false;
static int          arg_random_size           = 1000000;
static int          arg_random_seed           =       1;
static int          arg_random_outdegree      =       3;
static int          arg_random_priorities     =      20;
static int          arg_random_clustersize    =       0;
static int          arg_timeout               =       0;
static bool         arg_verify                = false;
static bool         arg_zielonka              = false;
static bool         arg_zielonka_sync         = false;
static int          arg_threads               = 0;
static bool         arg_mpi                   = false;
static int          arg_chunk_size            = -1;
static long long    arg_max_lifts             = -1;

static const double MB = 1048576.0;  // one megabyte

static volatile bool g_timed_out = false;

#ifdef POSIX
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
    return mstat.size*(getpagesize()/MB);
}
#else
static double get_vmsize()
{
    return 0;
}
#endif

static const char *bool_to_string(bool value)
{
    return value ? "true" : "false";
}

static const char *input_format_to_string(InputFormat input_format)
{
    switch (input_format)
    {
    case INPUT_NONE:      return "none";
    case INPUT_RAW:       return "raw";
    case INPUT_RANDOM:    return "random";
    case INPUT_PGSOLVER:  return "pgsolver";
    case INPUT_PBES:      return "pbes";
    default:              return "INVALID";
    }
}

//! Prints usage information (possible command line arguments).
static void print_usage(const char *argv0)
{
    printf( "Usage: %s [<options>] [<input>]\n\n", argv0);
    printf(
"General options:\n"
"  --help/-h              show this help message\n"
"  --verbosity/-v <level> message verbosity (0-6; default: 4)\n"
"  --quiet/-q             no messages (equivalent to -v0)\n"
"\n"
"Input:\n"
"  --input/-i <format>    input format: random, raw, PGSolver or PBES\n"
"  --size <int>           (random only) number of vertices\n"
"  --clustersize <int>    (random only) graph cluster size\n"
"  --outdegree <int>      (random only) average out-degree\n"
"  --priorities <int>     (random only) number of priorities\n"
"  --seed <int>           (random only) random number generator seed\n"
"\n"
"Preprocessing:\n"
"  --dual                 solve the dual game\n"
"  --reorder <desc>       reorder vertices before solving (comma-separated\n"
"                         list; possible values: bfs, dfs, reverse, shuffle)\n"
"  --propagate            propagate minimum priorities to predecessors\n"
"  --deloop               detect loops won by the controlling player\n"
"  --decycle              detect cycles won and controlled by a single player\n"
"  --scc                  solve strongly connected components individually\n"
"\n"
"Solving with Small Progress Measures:\n"
"  --lifting/-l <desc>    Small Progress Measures lifting strategy to use\n"
"                         ('help' shows available strategies and parameters)\n"
"  --lifting2/-L <desc>   The same but using the v2 algorithm implementation\n"
"  --alternate/-a         use Friedmann's two-sided solving approach\n"
"\n"
"Solving with Zielonka's recursive algorithm:\n"
"  --zielonka/-z          use Zielonka's recursive algorithm\n"
"  --threads <count>      solve concurrently using threads\n"
"  --mpi                  solve in parallel using MPI\n"
"  --chunk/-c <size>      (MPI only) chunk size for partitioning\n"
"  --sync                 (MPI only) use synchronized MPI algorithm\n"
"\n"
"Output:\n"
"  --dot/-d <file>        write parity game in GraphViz dot format to <file>\n"
"  --pgsolver/-p <file>   write parity game in PGSolver format to <file>\n"
"  --raw/-r <file>        write parity game in raw format to <file>\n"
"  --winners/-w <file>    write compact winners specification to <file>\n"
"  --strategy/-s <file>   write optimal strategy for both players to <file>\n"
"  --paritysol/-P <file>  write solution in PGSolver format to <file>\n"
"\n"
"Benchmarking/testing:\n"
"  --stats/-S             collect lifting statistics during SPM solving\n"
"  --timeout/-t <t>       abort solving after <t> seconds\n"
"  --maxlifts <n>         abort solving after <n> lifting attempts\n"
"  --verify/-V            verify solution after solving\n"
"  --hot/-H <file>        write 'hot' vertices in GraphViz format to <file>\n"
"  --debug/-D <file>      write solution in debug format to <file>\n");
}

//! Splits a string by a character, returning all non-empty (!) parts
std::vector<std::string> split(const std::string &s, char sep = ',')
{
    std::vector<std::string> values;
    for (size_t i = 0, j; i < s.size(); i = j + 1)
    {
        j = s.find(sep, i);
        if (j == std::string::npos) j = s.size();
        if (i < j) values.push_back(s.substr(i, j - i));
    }
    return values;
}

/*! Parses a long long integer in decimal notation followed by an optional
    exponent that signifies multiplications by a power of ten. For example,
    "12e3" represents 12,000.

    This function is similar to atoi() in that virtually no syntax checking
    is performed; if the argument is not formatted correctly, or exceeds the
    range of a long integer, the result will probably be wrong.

    If `str`cannot be parsed at all, false is result, and `res` is unchanged.
*/
static bool parse_long(const char *str, long long *res)
{
    int exp;
    switch (sscanf(str, "%llde%d", res, &exp))
    {
    case 2: for ( ; exp > 0; --exp) *res *= 10;
            for ( ; exp < 0; ++exp) *res /= 10;
    case 1: return true;
    }
    return false;
}

/*! Similar to the parse_long() above, but stores the result as an integer. */
static bool parse_int(const char *str, int *res)
{
    long long val;
    if (parse_long(str, &val))
    {
        *res = val;
        return true;
    }
    return false;
}

//! Parses command line arguments. Exits on failure.
static void parse_args(int argc, char *argv[])
{
    enum FileMode { text, binary, none } input_mode = none;

    static struct option long_options[] = {
        { "help",       no_argument,       NULL, 'h' },
        { "verbosity",  required_argument, NULL, 'v' },
        { "quiet",      no_argument,       NULL, 'q' },

        { "input",      required_argument, NULL, 'i' },
        { "size",       required_argument, NULL,  1  },
        { "clustersize",required_argument, NULL,  2  },
        { "outdegree",  required_argument, NULL,  3  },
        { "priorities", required_argument, NULL,  4  },
        { "seed",       required_argument, NULL,  5  },

        { "decycle",    no_argument,       NULL,  6  },
        { "deloop",     no_argument,       NULL,  7  },
        { "scc",        no_argument,       NULL,  8  },
        { "dual",       no_argument,       NULL,  9  },
        { "reorder",    required_argument, NULL, 10  },
        { "propagate",  no_argument,       NULL, 11  },

        { "lifting",    required_argument, NULL, 'l' },
        { "lifting2",   required_argument, NULL, 'L' },
        { "alternate",  no_argument,       NULL, 'a' },

        { "zielonka",   no_argument,       NULL, 'z' },
        { "threads",    required_argument, NULL, 12  },
        { "mpi",        no_argument,       NULL, 13  },
        { "chunk",      required_argument, NULL, 'c' },
        { "sync",       no_argument,       NULL, 14  },

        { "dot",        required_argument, NULL, 'd' },
        { "pgsolver",   required_argument, NULL, 'p' },
        { "raw",        required_argument, NULL, 'r' },
        { "winners",    required_argument, NULL, 'w' },
        { "strategy",   required_argument, NULL, 's' },
        { "paritysol",  required_argument, NULL, 'P' },

        { "stats",      no_argument,       NULL, 'S' },
        { "timeout",    required_argument, NULL, 't' },
        { "maxlifts",   required_argument, NULL, 15  },
        { "verify",     no_argument,       NULL, 'V' },
        { "hot",        required_argument, NULL, 'H' },
        { "debug",      required_argument, NULL, 'D' },
        { NULL,         no_argument,       NULL,  0  } };

    std::string options;
    for (struct option *opt = long_options; opt->name; ++opt)
    {
        if (opt->val > ' ')
        {
            options += (char)opt->val;
            if (opt->has_arg == required_argument) options += ':';
        }
    }

    for (;;)
    {
        int ch = getopt_long(argc, argv, options.c_str(), long_options, NULL);
        if (ch == -1) break;

        switch (ch)
        {
        case 'h':   /* help */
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
            break;

        case 'v':   /* set logger severity to (NONE - verbosity) */
            {
                int severity = Logger::LOG_NONE - atoi(optarg);
                if (severity > Logger::LOG_NONE)  severity = Logger::LOG_NONE;
                if (severity < Logger::LOG_DEBUG) severity = Logger::LOG_DEBUG;
                Logger::severity((Logger::Severity)severity);
            } break;

        case 'q':  /* set logger severity to NONE */
            Logger::severity(Logger::LOG_NONE);
            break;

        case 'i':   /* input format */
            if (strcasecmp(optarg, "random") == 0)
            {
                arg_input_format = INPUT_RANDOM;
            }
            else
            if (strcasecmp(optarg, "raw") == 0)
            {
                arg_input_format = INPUT_RAW;
                input_mode = binary;
            }
            else
            if (strcasecmp(optarg, "pgsolver") == 0)
            {
                arg_input_format = INPUT_PGSOLVER;
                input_mode = text;
            }
            else
            if (strcasecmp(optarg, "pbes") == 0)
            {
#ifdef WITH_MCRL2
                arg_input_format = INPUT_PBES;
                input_mode = binary;
#else
                printf("PBES input requires linking to mCRL2\n");
                exit(EXIT_FAILURE);
#endif
            }
            else
            {
                printf("Invalid input format: %s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;

        case 1:     /* random graph size */
            parse_int(optarg, &arg_random_size);
            break;

        case 2:     /* random graph cluster size */
            parse_int(optarg, &arg_random_clustersize);
            if (arg_random_clustersize < 2)
            {
                fprintf(stderr, "Invalid cluster size: %d (must be "
                                "greater than 1)\n", arg_random_clustersize);
                exit(EXIT_FAILURE);
            }
            break;

        case 3:     /* random graph out-degree */
            arg_random_outdegree = atoi(optarg);
            break;

        case 4:     /* random game number of priorities */
            arg_random_priorities = atoi(optarg);
            break;

        case 5:     /* random seed */
            arg_random_seed = atoi(optarg);
            break;

        case 6:     /* remove p-controlled i-cycles when p == i%2 */
            arg_decycle = true;
            break;

        case 7:     /* preprocess vertices with loops */
            arg_deloop = true;
            break;

        case 8:     /* decompose into strongly connected components */
            arg_scc_decomposition = true;
            break;

        case 9:     /* solve dual game */
            arg_solve_dual = true;
            break;

        case 10:    /* reorder vertices */
            arg_reordering = optarg;
            break;

        case 11:    /* enable priority propagation */
            arg_priority_propagation = true;
            break;

        case 'l':   /* Small Progress Measures v1 lifting strategy */
        case 'L':   /* Small Progress Measures v2 lifting strategy  */
            arg_spm_version = ch == 'l' ? 1 : ch == 'L' ? 2 : 0;
            arg_spm_lifting_strategy = optarg;
            if (strcasecmp(optarg, "help") == 0)
            {
                printf( "Available lifting strategies:\n\n%s",
                        LiftingStrategyFactory::usage() );
                exit(EXIT_SUCCESS);
            }
            break;

        case 'a':  /* Alternate SPM solver */
            arg_alternate = true;
            break;

        case 'z':   /* use Zielonka's algorithm instead of SPM */
            arg_zielonka = true;
            break;

        case 12:    /* concurrent solving */
            arg_threads = atoi(optarg);
            if (arg_threads < 1)
            {
                fprintf(stderr, "Invalid number of threads: %s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;

        case 13:    /* parallize solving with MPI */
            arg_mpi = true;
            break;

        case 'c':   /* use given chunk size */
            arg_chunk_size = atoi(optarg);
            if (arg_chunk_size < 1)
            {
                fprintf(stderr, "Invalid chunk size: %s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;

        case 14:    /* use synchronized algorithm */
            arg_zielonka_sync = true;
            break;

        case 'd':   /* dot output file */
            arg_dot_file = optarg;
            break;

        case 'p':   /* PGSolver output file */
            arg_pgsolver_file = optarg;
            break;

        case 'r':   /* raw output file */
            arg_raw_file = optarg;
            break;

        case 'w':   /* winners output file */
            arg_winners_file = optarg;
            break;

        case 's':   /* strategy output file */
            arg_strategy_file = optarg;
            break;

        case 'P':   /* "paritysol" (PGSolver --solonly format) output file */
            arg_paritysol_file = optarg;
            break;

        case 'S':   /* collect lifting statistics*/
            arg_collect_stats = true;
            break;

        case 't':   /* time limit (in seconds) */
            arg_timeout = atoi(optarg);
            break;

        case 15:   /* maximum lifts (in attempts) */
            parse_long(optarg, &arg_max_lifts);
            break;

        case 'V':   /* verify solution */
            arg_verify = true;
            break;

        case 'H':   /* debug hot vertices file */
            arg_hot_vertices_file = optarg;
            break;

        case 'D':   /* debug output file */
            arg_debug_file = optarg;
            break;

        case '?':
            {
                printf("Unrecognized option!\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
    }

    if (arg_input_format == INPUT_NONE)
    {
        printf("No input format specified! "
               "(Use --help to print usage information.)\n");
        exit(EXIT_FAILURE);
    }

    if (optind < argc)
    {
        /* Rmaining argument specifies an input file */
        if (input_mode == none || argc - optind > 1)
        {
            printf("Too many (non-option) arguments specified!\n");
            exit(EXIT_FAILURE);
        }
        const char *path = argv[optind];
        const char *mode = (input_mode == text) ? "rt" : "r";
        if (freopen(path, mode, stdin) == NULL)
        {
            printf("Could not open file \"%s\" for reading!\n", path);
            exit(EXIT_FAILURE);
        }
    }
}

/*! Write summary of winners. For each vertex, a single character is printed:
    'E' or 'O', depending on whether player Even or Odd wins the parity game
    starting from this vertex. */
static void write_winners( std::ostream &os, const ParityGame &game,
                           const ParityGame::Strategy &strategy )
{
    verti next_newline = 80;
    for (verti v = 0; v < game.graph().V(); ++v)
    {
        if (v == next_newline)
        {
            os << '\n';
            next_newline += 80;
        }
        ParityGame::Player winner = game.winner(strategy, v);
        os << ( (winner == ParityGame::PLAYER_EVEN) ^ arg_solve_dual ? 'E' :
                (winner == ParityGame::PLAYER_ODD)  ^ arg_solve_dual ? 'O' :
                                                                       '?' );
    }
    os << '\n';
}

/*! Write strategy description. For each vertex won by its player, a single line
    is printed, of the form: v->w (where v and w are 0-based vertex indices). */
static void write_strategy( std::ostream &os,
                            const ParityGame::Strategy &strategy )
{
    for (verti v = 0; v < (verti)strategy.size(); ++v)
    {
        if (strategy[v] != NO_VERTEX) os << v << "->" << strategy[v] << '\n';
    }
}

/*! Write solution in PGSolver --solonly format, which can be parsed by tools
    like MLSsolver. */
static void write_paritysol( std::ostream &os,
                             const ParityGame &game,
                             const ParityGame::Strategy &strategy )
{
    const StaticGraph &graph = game.graph();
    const verti V = graph.V();
    assert(strategy.size() == V);
    os << "paritysol " << (long long)V - 1 << ";\n";
    for (verti v = 0; v < V; ++v)
    {
        os << v << ' ' << (int)game.winner(strategy, v);
        if (strategy[v] != NO_VERTEX) os << ' ' << strategy[v];
        os << ";\n";
    }
}

/*! Write a subgraph containing hot vertices (vertices that were lifted at
   least `threshold` times) in GraphViz format to given output stream. */
static void write_hot_vertices( std::ostream &os, const ParityGame &game,
    const LiftingStatistics &stats, long long threshold )
{
    const StaticGraph &graph = game.graph();
    std::set<std::pair<verti, verti> > edges;
    std::set<verti> vertices, hot;
    for (verti v = 0; v < graph.V(); ++v)
    {
        if (stats.lifts_succeeded(v) >= threshold)
        {
            hot.insert(v);
            vertices.insert(v);
            for ( StaticGraph::const_iterator it = graph.succ_begin(v);
                    it != graph.succ_end(v); ++it )
            {
                vertices.insert(*it);
                edges.insert(std::make_pair(v, *it));
            }
            for ( StaticGraph::const_iterator it = graph.pred_begin(v);
                    it != graph.pred_end(v); ++it )
            {
                vertices.insert(*it);
                edges.insert(std::make_pair(*it, v));
            }
        }
    }
    os << "digraph {\n";
    for ( std::set<verti>::const_iterator it = vertices.begin();
            it != vertices.end(); ++it )
    {
        os << *it << " [shape=" << (game.player(*it) ? "box": "diamond")
            << ", label=\"" << game.priority(*it) << "\\n(" << *it << ")\"";
        if (hot.count(*it)) os << ", style=\"filled\"";
        os << "]\n";
    }
    for ( std::set<verti>::const_iterator it = hot.begin();
            it != hot.end(); ++it )
    {
        os << *it << "->l" << *it << " [arrowhead=none];\n"
           << "l" << *it << " [shape=plaintext, label=\""
           << stats.lifts_succeeded(*it) << " /\\n"
           << stats.lifts_attempted(*it) << "\"]\n";
    }
    for ( std::set<std::pair<verti, verti> >::const_iterator
            it = edges.begin(); it != edges.end(); ++it )
    {
        os << it->first << "->" << it->second << ";\n";
    }
    os << "}\n";
}

//! Reads/generates parity game as specified by the user.
bool read_input(ParityGame &game)
{
    switch (arg_input_format)
    {
    case INPUT_RANDOM:
        Logger::message("## config.random.vertices    = %10d",
                        arg_random_size);
        Logger::message("## config.random.clustersize = %10d",
                        arg_random_clustersize);
        Logger::message("## config.random.outdegree   = %10d",
                        arg_random_outdegree);
        Logger::message("## config.random.priorities  = %10d",
                        arg_random_priorities);
        Logger::message("## config.random.seed        = %10d",
                        arg_random_seed);
        game.make_random(
            arg_random_size, arg_random_clustersize, arg_random_outdegree,
            StaticGraph::EDGE_BIDIRECTIONAL, arg_random_priorities );
        return true;

    case INPUT_RAW:
        Logger::info("Reading raw input...");
        game.read_raw(std::cin);
        return true;

    case INPUT_PGSOLVER:
        Logger::info("Reading PGSolver input...");
        game.read_pgsolver(std::cin);
        return !game.empty();

    case INPUT_PBES:
        Logger::info("Generating parity game from PBES input....");
        game.read_pbes("");
        return true;

    case INPUT_NONE:
        return false;
    }

    return false;
}

//! Writes parity game and solution data as specified by the user.
void write_output( const ParityGame &game,
    const ParityGame::Strategy &strategy = ParityGame::Strategy(),
    LiftingStatistics *stats = NULL )
{
    /* Write dot file */
    if (!arg_dot_file.empty())
    {
        if (arg_dot_file == "-")
        {
            game.write_dot(std::cout);
            if (!std::cout) Logger::error("Writing failed!");
        }
        else
        {
            Logger::info( "Writing GraphViz dot game description to file %s...",
                          arg_dot_file.c_str() );
            std::ofstream ofs(arg_dot_file.c_str());
            game.write_dot(ofs);
            if (!ofs) Logger::error("Writing failed!");
        }
    }

    /* Write PGSolver file */
    if (!arg_pgsolver_file.empty())
    {
        if (arg_pgsolver_file == "-")
        {
            game.write_pgsolver(std::cout);
        }
        else
        {
            Logger::info( "Writing PGSolver game description to file %s...",
                          arg_pgsolver_file.c_str() );
            std::ofstream ofs(arg_pgsolver_file.c_str());
            game.write_pgsolver(ofs);
            if (!ofs) Logger::error("Writing failed!");
        }
    }

    /* Write raw parity game file */
    if (!arg_raw_file.empty())
    {
        if (arg_raw_file == "-")
        {
            game.write_raw(std::cout);
        }
        else
        {
            Logger::info( "Writing raw game description to file %s...",
                          arg_raw_file.c_str() );
            std::ofstream ofs(arg_raw_file.c_str());
            game.write_raw(ofs);
            if (!ofs) Logger::error("Writing failed!");
        }
    }

    /* Write winners file */
    if (!arg_winners_file.empty() && !strategy.empty())
    {
        if (arg_winners_file == "-")
        {
            write_winners(std::cout, game, strategy);
            if (!std::cout) Logger::error("Writing failed!");
        }
        else
        {
            Logger::info( "Writing winners to file %s...",
                          arg_winners_file.c_str() );
            std::ofstream ofs(arg_winners_file.c_str());
            write_winners(ofs, game, strategy);
            if (!ofs) Logger::error("Writing failed!");
        }
    }

    /* Write strategy file */
    if (!arg_strategy_file.empty() && !strategy.empty())
    {
        if (arg_strategy_file == "-")
        {
            write_strategy(std::cout, strategy);
            if (!std::cout) Logger::error("Writing failed!");
        }
        else
        {
            Logger::info( "Writing strategy to file %s...",
                          arg_strategy_file.c_str() );
            std::ofstream ofs(arg_strategy_file.c_str());
            write_strategy(ofs, strategy);
            if (!ofs) Logger::error("Writing failed!");
        }
    }

    /* Write paritysol file */
    if (!arg_paritysol_file.empty() && !strategy.empty())
    {
        if (arg_paritysol_file == "-")
        {
            write_paritysol(std::cout, game, strategy);
            if (!std::cout) Logger::error("Writing failed!");
        }
        else
        {
            Logger::info( "Writing PGSolver solution description to file %s...",
                          arg_paritysol_file.c_str() );
            std::ofstream ofs(arg_paritysol_file.c_str());
            write_paritysol(ofs, game, strategy);
            if (!ofs) Logger::error("Writing failed!");
        }
    }

    /* Write hot vertices file */
    if (stats != NULL && !arg_hot_vertices_file.empty())
    {
        if (!split(arg_reordering).empty())
        {
            Logger::error("Vertex reordering has distorted vertex indices!");
            // FIXME: to fix this, I should re-order vertex statistics after
            //        solving (see inv_perm in main()).
        }
        // FIXME: make threshold a command-line parameter?
        long long threshold = stats->lifts_succeeded()/1000;
        if (arg_hot_vertices_file == "-")
        {
            write_hot_vertices(std::cout, game, *stats, threshold);
        }
        else
        {
            Logger::info( "Writing hot vertices to file %s...",
                          arg_hot_vertices_file.c_str() );
            std::ofstream ofs(arg_hot_vertices_file.c_str());
            write_hot_vertices(ofs, game, *stats, threshold);
            if (!ofs) Logger::error("Writing failed!");
        }
    }

    /* Write debug file */
    if (!arg_debug_file.empty())
    {
        if (arg_debug_file == "-")
        {
            game.write_debug(strategy, std::cout);
        }
        else
        {
            Logger::info( "Writing debug info to file %s...",
                          arg_debug_file.c_str() );
            std::ofstream ofs(arg_debug_file.c_str());
            game.write_debug(strategy, ofs);
            if (!ofs) Logger::error("Writing failed!");
        }
    }
}

#ifdef POSIX
static void alarm_handler(int sig)
{
    if (sig == SIGALRM && !g_timed_out)
    {
        g_timed_out = true;
        Abortable::abort_all();
    }
}

static void set_timeout(int t)
{
    /* Set handler for alarm signal */
    struct sigaction act;
    act.sa_handler = &alarm_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    /* Schedule alarm signal */
    int res = sigaction(SIGALRM, &act, NULL);
    if (res != 0)
    {
        Logger::warn("Couldn't install signal handler.");
    }
    else
    {
        alarm(t);
    }
}
#else
static void set_timeout(int t)
{
    (void)t;  // unused
    Logger::warn("Time-out not available.");
}
#endif

//! Application entry point.
int main(int argc, char *argv[])
{
    Logger::severity(Logger::LOG_WARN);

#ifdef WITH_MPI
    MPI::Init(argc, argv);
    mpi_rank = MPI::COMM_WORLD.Get_rank();
    mpi_size = MPI::COMM_WORLD.Get_size();
#endif

    parse_args(argc, argv);

    if (arg_mpi)
    {
        // Make stderr line-buffered to avoid interleaving of log output
        // from different MPI processes.
        static char stderr_buf[1024];
        setvbuf(stderr, stderr_buf, _IOLBF, sizeof(stderr_buf));
    }

    Logger::message( "## config.input = %s",
                     input_format_to_string(arg_input_format) );

    // Always seed the RNG, even if input is not randomly generated,
    // so other randomized algorithms like --reorder shuffle can be seeded.
    srand(arg_random_seed);

    ParityGame game;
    if (!read_input(game))
    {
        Logger::fatal("Couldn't parse parity game from input!");
    }
    assert(game.proper());

    // Do priority compression at the start:
    game.compress_priorities();

    Logger::message("## config.dual = %s", bool_to_string(arg_solve_dual));
    if (arg_solve_dual)
    {
        Logger::info("Switching to dual game...");
        game.make_dual();
    }

    bool failed = true;

    if (arg_spm_lifting_strategy.empty() && !arg_zielonka)
    {
        // Don't solve; just convert data.
        write_output(game);
        failed = false;
    }
    else
    {
        std::auto_ptr<LiftingStatistics> stats;

        Logger::message("## config.threads = %d",     arg_threads);

        if (arg_threads)
        {
#ifndef WITH_THREADS
            Logger::fatal("Thread support was not compiled in!");
#else
            omp_set_num_threads(arg_threads);
#endif
        }

        Logger::message("## config.mpi = %s", bool_to_string(arg_mpi));
#ifndef WITH_MPI
        if (arg_mpi) Logger::fatal("MPI support was not compiled in!");
#endif

#ifdef WITH_MPI
        VertexPartition *vpart = NULL;
        if (arg_mpi)
        {
            Logger::message("## config.mpi.chunk = %d", arg_chunk_size);
            Logger::message("## config.mpi.sync  = %s",
                            bool_to_string(arg_zielonka_sync));
            vpart = new VertexPartition( mpi_size, arg_chunk_size > 0
                ? arg_chunk_size : (game.graph().V() + mpi_size - 1)/mpi_size );
        }
#endif

        // Create appropriate solver factory:
        std::auto_ptr<ParityGameSolverFactory> solver_factory;

        if (arg_zielonka && !arg_spm_lifting_strategy.empty())
        {
            Logger::fatal("Multiple solving algorithms selected!\n");
        }

        // Allocate lifting strategy:
        if (!arg_spm_lifting_strategy.empty())
        {
            Logger::message("## config.solver = %s",
                            arg_spm_version < 2 ? "spm" : "spm2");
            Logger::message("## config.spm.alternate = %s",
                            bool_to_string(arg_alternate));
            Logger::message("## config.spm.strategy = %s",
                            arg_spm_lifting_strategy.c_str());
            Logger::message("## config.spm.count_lifts = %s",
                            bool_to_string(arg_collect_stats));

            LiftingStrategyFactory *spm_strategy = 
                LiftingStrategyFactory::create(arg_spm_lifting_strategy);

            if (!spm_strategy)
            {
                Logger::fatal( "Invalid lifting strategy description: %s",
                               arg_spm_lifting_strategy.c_str() );
            }

            if (!spm_strategy->supports_version(arg_spm_version))
            {
                Logger::fatal( "Lifting strategy does not SPM version %d. "
                                "(Try %s %s instead.)", arg_spm_version,
                                arg_spm_version == 1 ? "-L" : "-l",
                                arg_spm_lifting_strategy.c_str() );
            }

            if (arg_collect_stats)
            {
                stats.reset(new LiftingStatistics(game, arg_max_lifts));
            }

            if (!arg_mpi)
            {
                solver_factory.reset(new SmallProgressMeasuresSolverFactory(
                    spm_strategy, arg_spm_version, arg_alternate, stats.get() ));
            }
#ifdef WITH_MPI
            else
            {
                if (arg_alternate)
                {
                    Logger::fatal( "MPI SPM solver does not support "
                                   "two-sided approach (option -a)" );
                }
                if (arg_spm_version != 1)
                {
                    Logger::fatal("MPI SPM solver only supports SPM version 1");
                }
                solver_factory.reset(new MpiSpmSolverFactory(
                    spm_strategy, vpart, stats.get() ));
            }
#endif
            spm_strategy->deref();
        }

        // Create recursive solver factory if requested:
        if (arg_zielonka)
        {
            Logger::message("## config.solver = zielonka");
            if (!arg_mpi)
            {
                if (!arg_threads)
                {
                    solver_factory.reset(new RecursiveSolverFactory());
                }
#ifdef WITH_THREADS
                else
                {
                    solver_factory.reset(new ConcurrentRecursiveSolverFactory());
                }
#endif
            }
#ifdef WITH_MPI
            else
            {
                solver_factory.reset(
                    new MpiRecursiveSolverFactory(!arg_zielonka_sync, vpart) );
            }
#endif
        }

#ifdef WITH_MPI
        if (vpart)
        {
            vpart->deref();
            vpart = NULL;
        }
#endif

        Logger::message("## config.timeout = %d s", arg_timeout);
        if (arg_timeout > 0) set_timeout(arg_timeout);

        // Vertex reordering:
        std::vector<std::string> parts = split(arg_reordering);
        std::vector<verti> perm;
        if (!parts.empty())
        {
            // FIXME: this should probably count towards solving time
            Logger::message("## config.reordering = %s", arg_reordering.c_str());
            const verti V = game.graph().V();
            std::vector<verti> next_perm(V);
            for (size_t i = 0; i < parts.size(); ++i)
            {
                if (parts[i] == "bfs")
                {
                    Logger::info("Reordering vertices by "
                                 "breadth-first search order...");
                    get_bfs_order(game.graph(), next_perm);
                }
                else
                if (parts[i] == "dfs")
                {
                    Logger::info("Reordering vertices by "
                                 "depth-first search order...");
                    get_dfs_order(game.graph(), next_perm);
                }
                else
                if (parts[i] == "reverse" || parts[i] == "rev")
                {
                    Logger::info("Reordering vertices by reverse index...");
                    for (verti v = 0; v < V; ++v) next_perm[v] = V - v - 1;
                }
                else
                if (parts[i] == "shuffle")
                {
                    Logger::info("Reordering vertices randomly...");
                    for (verti v = 0; v < V; ++v) next_perm[v] = v;
                    shuffle_vector(next_perm);
                }
                else
                {
                    Logger::fatal("Invalid graph reordering: \"%s\"",
                                  parts[i].c_str());
                }
                game.shuffle(next_perm);

                if (i == 0)
                {
                    perm = next_perm;
                }
                else
                {
                    std::vector<verti> old = perm;
                    for (verti v = 0; v < V; ++v) perm[v] = next_perm[old[v]];
                }
            }
        }

        {
            // FIXME: this should probably count towards solving time
            // FIXME: it might only be useful when solve with SPM solver
            Logger::info("Preprocessing graph...");
            edgei old_edges = game.graph().E();
            SmallProgressMeasuresSolver::preprocess_game(game);
            edgei rem_edges = old_edges - game.graph().E();
            Logger::info("Removed %d edge%s...", rem_edges, rem_edges == 1 ? "" : "s");
        }

        /* Note: priority propagation is done after preprocessing, because
                 it benefits from removed loops (since priorities can only be
                 propagated to vertices without loops). */
        Logger::message("## config.propagate = %s",
                        bool_to_string(arg_priority_propagation));
        if (arg_priority_propagation)
        {
            Logger::info("Propagating priorities...");
            long long updates = game.propagate_priorities();
            Logger::info("Reduced summed priorities by %lld.", updates);
            game.compress_priorities();
        }

        /* Print some game info: */
        Logger::message("## game.vertices   = %12lld", (long long)game.graph().V());
        Logger::message("## game.edges      = %12lld", (long long)game.graph().E());
        Logger::message("## game.edge_ratio = %.10lf",
            (double)count_forward_edges(game.graph())/game.graph().E() );
        Logger::message("## game.priorities = %12d", game.d());
        {
            long long sum = 0;
            for (int p = 0; p < game.d(); ++p)
            {
                verti count = game.cardinality(p);
                Logger::info("  %2d occurs %d times", p, count);
                sum += (long long)p*count;
            }
            Logger::message( "## game.average_priority = %.10lf",
                            (double)sum/game.graph().V() );
        }

        if (arg_spm_version)
        {
            /* Calculate upper bound on maximum number of lifts (when solving
               as normal game, N, or dual game, D) based only on how often
               each priority occurs in the game.

               This is most meaningful when the game is a single SCC and
               therefore each vertex is reachable from every other vertex.
            */
            Decimal N(0), D(0);
            for (int p = 0; p < game.d(); ++p)
            {
                Decimal n(1), d(1);
                for (int q = 0; q <= p; ++q)
                {
                    Decimal &s = (q%2 == 1) ? n : d;
                    s = s + s*Decimal(game.cardinality(q));
                }
                N = N + n*Decimal(game.cardinality(p));
                D = D + d*Decimal(game.cardinality(p));
            }
            Logger::message("## max_lifts.normal = %s", N.c_str());
            Logger::message("## max_lifts.dual   = %s", D.c_str());
        }

        /* Add preprocessors which wrap the current solver factory.

           Note that wrapping is done inside-out: the last wrapper added will
           run first.  The proper order is: Deloop -> Decycle -> Component.
        */
        Logger::message( "## config.decompose = %s",
                         bool_to_string(arg_scc_decomposition) );
        if (arg_scc_decomposition)
        {
            solver_factory.reset(
                new ComponentSolverFactory(*solver_factory.release()) );
        }
        Logger::message( "## config.decycle = %s",
                         bool_to_string(arg_decycle) );
        if (arg_decycle)
        {
            solver_factory.reset(
                new DecycleSolverFactory(*solver_factory.release()) );
        }
        Logger::message( "## config.deloop = %s",
                         bool_to_string(arg_deloop) );
        if (arg_deloop)
        {
            // N.B. current implementation of the DeloopSolver assumes
            //      the game has been preprocessed as done above!
            solver_factory.reset(
                new DeloopSolverFactory(*solver_factory.release()) );
        }

        Timer timer;
        Logger::info("Starting solve...");

        // Create solver instance:
        assert(solver_factory.get() != NULL);
        std::auto_ptr<ParityGameSolver> solver(solver_factory->create(game));

        // Now solve the game:
        ParityGame::Strategy strategy = solver->solve();

#ifdef WITH_MPI
        if (mpi_rank > 0)
        {
            // Join processes here, to avoid writing output multiple times.
            MPI::Finalize();
            return EXIT_SUCCESS;
        }
#endif

        failed = strategy.empty();
        if (failed)
        {
            if (solver->aborted())
            {
                Logger::error("time limit exceeded!");
                Logger::message("## solution.result = aborted");
            }
            else
            {
                Logger::error("solving failed!");
                Logger::message("## solution.result = failure");
            }
        }
        else
        {
            Logger::message("## solution.result = success");
        }

        // Print some statistics
        Logger::message("## solution.time   = %10.3f s", timer.elapsed());
        Logger::message("## solution.memory = %10.3f MB", get_vmsize());

        if (stats.get() != NULL)
        {
            long long lifts_total       = stats->lifts_attempted();
            long long lifts_successful  = stats->lifts_succeeded();
            long long lifts_failed      = lifts_total - lifts_successful;

            Logger::message("## lifts.failure = %12lld", lifts_failed);
            Logger::message("## lifts.success = %12lld", lifts_successful);
            Logger::message("## lifts.total   = %12lld", lifts_total);
            /*
            Logger::message( "Minimum lifts required:       %12lld",
                             0LL);  // TODO
            */
        }

        if (!perm.empty())
        {
            // Restore permuted vertices:
            std::vector<verti> inv_perm(perm.size());
            for (size_t i = 0; i < perm.size(); ++i) inv_perm[perm[i]] = i;
            game.shuffle(inv_perm);
            if (!strategy.empty())
            {
                std::vector<verti> new_strategy(strategy.size());
                for (size_t i = 0; i < strategy.size(); ++i)
                {
                    new_strategy[inv_perm[i]] =
                        (strategy[i] == NO_VERTEX) ? strategy[i]
                                                   : inv_perm[strategy[i]];
                }
                strategy.swap(new_strategy);
            }
        }

        if (!failed && arg_verify)
        {
            Timer timer;
            verti error;

            Logger::info("Starting verification...");
            if (game.verify(strategy, &error))
            {
                Logger::message("## verification = success");
            }
            else
            {
                failed = true;
                // Complain loudly so this message gets noticed:
                Logger::error("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                Logger::error("!!                                    !!");
                Logger::error("!!        Verification failed!        !!");
                Logger::error("!!                                    !!");
                Logger::error("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                Logger::error("Error at vertex: %d", (int)error);
                Logger::message("## verification = failure");
            }
            Logger::message("## verification.time = %10.3f s", timer.elapsed());
        }
        else
        {
            Logger::message("## verification = skipped");
        }
        write_output(game, strategy, stats.get());
    }

    Logger::info("Exiting.");

#ifdef WITH_MPI
    MPI::Finalize();
#endif

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
