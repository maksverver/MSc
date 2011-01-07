// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "MpiRecursiveSolver.h"
#include "GamePartition.h"
#include <assert.h>
#include <algorithm>
#include <set>
#include <utility>

//! MPI tags used to identify different types of messages exchanged through MPI.
enum MpiTags {
    TAG_VERTEX, TAG_PROBE, TAG_TERM
};

#if 0
#include <sstream>

/* For debugging: returns the internal vertex set of the partition as a string,
   or a subset of according to `sel' if it is non-empty. */
static std::string str( const GamePartition &part,
                        const std::vector<char> &sel = std::vector<char>() )
{
    std::ostringstream os;
    bool first = true;
    os << "{ ";
    for ( GamePartition::const_iterator it = part.begin();
          it != part.end(); ++it )
    {
        if (sel.empty() || sel[*it])
        {
            if (first) first = false; else os << ", ";
            os << part.global(*it);
        }
    }
    os << " }";
    return os.str();
}
#endif

/*! Returns a list of indices at which `incl' is zero. */
static std::vector<verti> collect_complement(std::vector<char> &incl)
{
    std::vector<verti> res;
    for (size_t i = 0; i < incl.size(); ++i)
    {
        if (!incl[i]) res.push_back((verti)i);
    }
    return res;
}

//! Returns the sum of local values of all MPI processes:
static int mpi_sum(int local_value)
{
    int global_sum = 0;
    MPI::COMM_WORLD.Allreduce(&local_value, &global_sum, 1, MPI_INT, MPI_SUM);
    return global_sum;
}

//! Returns whether `local_value' is true in any of the MPI processes:
static bool mpi_or(bool local_value)
{
    return mpi_sum((int)local_value) != 0;
}

//! Returns whether `local_value' is true in all of the MPI processes:
static bool mpi_and(bool local_value)
{
    return mpi_sum((int)!local_value) == 0;
}

MpiRecursiveSolver::MpiRecursiveSolver(const ParityGame &game)
    : ParityGameSolver(game)
{
    // Ensure mpi_rank and mpi_size have been initialized:
    assert(mpi_size > 0 && mpi_rank >= 0 && mpi_rank < mpi_size);

    // Sanity check: we assume vertex indices are unsigned integers:
    assert(sizeof(int) == sizeof(verti));
}

MpiRecursiveSolver::~MpiRecursiveSolver()
{
}

ParityGame::Strategy MpiRecursiveSolver::solve()
{
    const verti V = game().graph().V();

    // Initialize stragegy
    strategy_ = ParityGame::Strategy(V, NO_VERTEX);

    {
        // Select vertices for my initial partition of the game graph:
        std::vector<verti> verts;
        verts.reserve((V + mpi_size - 1)/mpi_size);
        for (verti v = mpi_rank; v < V; v += mpi_size)
        {
            assert(worker(v) == mpi_rank);
            verts.push_back(v);
        }

        // Solve the game:
        GamePartition partition(game(), verts);
        solve(partition, 0);
    }

    // Collect resulting strategy
    ParityGame::Strategy result;
    result.swap(strategy_);
    if (aborted())
    {
        result.clear();
    }
    else
    if (mpi_rank == 0)
    {
        info("Combining strategy...");
        for (verti v = 0; v < V; ++v)
        {
            int i = worker(v);
            if (i != mpi_rank)
            {
                int val = -1;
                MPI::COMM_WORLD.Recv(&val, 1, MPI_INT, i, 0);
                result[v] = (verti)val;
            }
        }
    }
    else  // mpi_rank > 0
    {
        for (verti v = 0; v < V; ++v)
        {
            if (worker(v) == mpi_rank)
            {
                int val = result[v];
                MPI::COMM_WORLD.Send(&val, 1, MPI_INT, 0, 0);
            }
        }
        result.clear();
    }

    return result;
}

void MpiRecursiveSolver::solve(GamePartition &part, int min_prio)
{
    const ParityGame::Player player   = (ParityGame::Player)(min_prio%2);
    const ParityGame::Player opponent = (ParityGame::Player)(1 - min_prio%2);

    assert(min_prio < part.game().d());

    if (part.game().d() - min_prio == 1)
    {
        //Logger::debug("part=%s min_prio=%d", str(part).c_str(), min_prio);

        // Only one priority left; construct trivial strategy.
        for ( GamePartition::const_iterator it = part.begin();
              it != part.end(); ++it )
        {
            const verti v = part.global(*it);
            if (game_.player(v) == player)
            {
                // I win; pick arbitrary successor inside the subgame
                strategy_[v] = part.global(*part.game().graph().succ_begin(*it));
            }
            else
            {
                    // Opponent loses
                strategy_[v] = NO_VERTEX;
            }
        }
        return;
    }

    do
    {
        //Logger::debug("part=%s min_prio=%d", str(part).c_str(), min_prio);

        const verti V = part.total_size();
        if (mpi_rank == 0 && aborted())
        {
            MPI::COMM_WORLD.Abort(1);
            return;
        }

        // Find attractor set of vertices with minimum priority
        std::vector<char> min_prio_attr(V, 0);
        std::deque<verti> min_prio_attr_queue;
        for (verti v = 0; v < V; ++v )
        {
            if (part.game().priority(v) == min_prio)
            {
                min_prio_attr[v] = 1;
                min_prio_attr_queue.push_back(v);
            }
        }
        //debug("|min_prio|=%d", mpi_sum((int)min_prio_attr_queue.size()));
        make_attractor_set(part, player, min_prio_attr, min_prio_attr_queue, true);
        //debug("|min_prio_attr|=%d", mpi_sum((int)set_size(part, min_prio_attr)));
        std::vector<verti> unsolved = collect_complement(min_prio_attr);

        // Check if attractor set covers the entire game:
        if (mpi_and(unsolved.empty())) break;

        // Solve subgame with remaining vertices and fewer priorities:
        GamePartition subpart(part, unsolved);
        solve(subpart, min_prio + 1);

        // Find attractor set of vertices lost to opponent in subgame:
        std::vector<char> lost_attr(V, 0);
        std::deque<verti> lost_attr_queue;
        for ( GamePartition::const_iterator it = subpart.begin();
              it != subpart.end(); ++it )
        {
            verti w = subpart.global(*it);
            assert(game_.winner(strategy_, w) == opponent);
            verti v = part.local(w);
            lost_attr[v] = 1;
            lost_attr_queue.push_back(v);
        }
        //debug("|lost|=%d", (int)lost_attr_queue.size());

        // Check if opponent's winning set is empty:
        if (mpi_and(lost_attr_queue.empty())) break;

        // Create subgame with vertices not yet lost to opponent:
        make_attractor_set(part, opponent, lost_attr, lost_attr_queue);
        //debug("|lost_attr|=%d", (int)set_size(part, lost_attr));

        std::vector<verti> not_lost = collect_complement(lost_attr);
        GamePartition(part, not_lost).swap(part);

    } while (mpi_or(!part.empty()));

    // If we get here, then the opponent's winning set was empty; the strategy
    // for most vertices has already been initialized, except for those with
    // minimum priority. Since the whole game is won by the current player, it
    // suffices to pick an arbitrary successor for these vertices:
    for (GamePartition::const_iterator it = part.begin(); it != part.end(); ++it)
    {
        const verti v = part.global(*it);
        if (game_.priority(v) == min_prio)
        {
            if (game_.player(v) == player)  // player wins
            {
                strategy_[v]
                    = part.global(*part.game().graph().succ_begin(*it));
            }
            else  // opponent loses
            {
                strategy_[v] = NO_VERTEX;
            }
        }
        else
        {
            assert(game_.priority(v) >= min_prio);
            assert(game_.winner(strategy_, v) == player);
        }
    }
}

/*! Sends local vertex i to every worker that has a predecessor in its internal
    vertex set (using the global vertex index), but only once per worker.
    Afterwards, receives new vertices from other processes, ands add them to the
    local queue. */
void MpiRecursiveSolver::notify_others( const GamePartition &part, verti i,
    std::deque<verti> &queue, std::vector<char> &attr,
    MPI::Prequest &req, const verti &req_val,
    int &num_send, int &num_recv )
{
    const StaticGraph &graph = part.game().graph();
    verti v = part.global(i);
    std::vector<bool> recipients(mpi_size);
    for ( StaticGraph::const_iterator it = graph.pred_begin(i);
            it != graph.pred_end(i); ++it )
    {
        recipients[worker(part.global(*it))] = true;
    }
    for ( StaticGraph::const_iterator it = graph.succ_begin(i);
            it != graph.succ_end(i); ++it )
    {
        recipients[worker(part.global(*it))] = true;
    }
    for (int dest = 0; dest < mpi_size; ++dest)
    {
        if (recipients[dest] && dest != mpi_rank)
        {
            //debug("sending %d to %d", v, dest);
            MPI::COMM_WORLD.Send(&v, 1, MPI_INT, dest, TAG_VERTEX);
            ++num_send;
        }
    }

    // Receive pending vertex updates:
    while (req.Test())
    {
        //debug("received %d", req_val);
        i = part.local(req_val);
        assert(!attr[i]);
        attr[i] = true;
        queue.push_back(i);
        ++num_recv;
        req.Start();
    }
}

void MpiRecursiveSolver::make_attractor_set(
    const GamePartition &part, ParityGame::Player player,
    std::vector<char> &attr, std::deque<verti> &queue,
    bool quick_start )
{
    /*
    debug( "enter make_attractor_set(%s, %d) in %s",
           str(part, attr).c_str(), (int)quick_start, str(part).c_str());
    */

    const StaticGraph &graph = part.game().graph();

    // Uses Friedemann Mattern's four-counter method for termination detection.
    // All processes keep track of the number of sent and received messages.
    // When idle, the first process sends a probe that is circulated to other
    // processes when they are idle, accumulating the total number of messages
    // sent and received. This has to be done twice in order to confirm global
    // termination, at which point the first process sends a termination signal
    // to the other processes.

    bool send_probe = (mpi_rank == 0);
    int num_send = 0, num_recv = 0;
    int total_send = 0, total_recv = 0;

    // Set up asynchronous receive buffers for the three different messages:
    verti vertex_val;
    int probe_val[2] = { 0, 0 };
    MPI::Prequest reqs[3] = {
        MPI::COMM_WORLD.Recv_init( &vertex_val, 1, MPI_INT,
                                   MPI::ANY_SOURCE, TAG_VERTEX ),

        MPI::COMM_WORLD.Recv_init( &probe_val[0], 2, MPI_INT,
                                   MPI::ANY_SOURCE, TAG_PROBE ),

        MPI::COMM_WORLD.Recv_init( NULL, 0, MPI_INT, 0, TAG_TERM )
    };
    MPI::Prequest::Startall(3, reqs);

    // When quick starting, processes are aware of each other's initial vertex
    // sets. If not, then we must first transmit the contents of the initial
    // set to the relevant other processes.
    if (!quick_start)
    {
        // N.B. queue.size() cannot be calculated inside the loop, because
        //      notify_others may push new, non-local vertices in the queue!
        size_t n = queue.size();
        for (size_t i = 0; i < n; ++i)
        {
            notify_others( part, queue[i], queue, attr, reqs[TAG_VERTEX],
                           vertex_val, num_send, num_recv );
        }
    }

    for (;;)
    {
        // Active: process queued vertices
        //debug("active");
        while (!queue.empty())
        {
            const verti w = queue.front();
            queue.pop_front();
            for ( StaticGraph::const_iterator it = graph.pred_begin(w);
                  it != graph.pred_end(w); ++it )
            {
                const verti v = *it;

                if (attr[v]) continue;

                // FIXME: this is a bit of a hack! The intent is to process
                //        internal vertices only, but `part' doesn't store
                //        any info to quickly decide this.
                if (worker(part.global(v)) != mpi_rank) continue;

                if (part.game().player(v) == player)
                {
                    // Store strategy for player-controlled vertex:
                    strategy_[part.global(v)] = part.global(w);
                }
                else  // opponent-controlled vertex
                {
                    // Can the opponent keep the token out of the attractor set?
                    for (StaticGraph::const_iterator jt = graph.succ_begin(v);
                        jt != graph.succ_end(v); ++jt)
                    {
                        if (!attr[*jt]) goto skip_v;
                    }

                    // Store strategy for opponent-controlled vertex:
                    strategy_[part.global(v)] = NO_VERTEX;
                }
                // Add vertex v to the attractor set:
                attr[v] = true;
                queue.push_back(v);
                //debug("added %d to attractor set", part.global(v));
                notify_others( part, v, queue, attr, reqs[TAG_VERTEX],
                               vertex_val, num_send, num_recv );

            skip_v:
                continue;
            }
        }

        // Idle: wait for a message to respond to.
        //debug("idle");
        if (send_probe)
        {
            if (mpi_size < 2) goto terminated;
            MPI::COMM_WORLD.Send(probe_val, 2, MPI_INT, 1, TAG_PROBE);
            send_probe = false;
            //debug("sent probe");
        }
        while (queue.empty())
        {
            switch (MPI::Request::Waitany(3, reqs))
            {
            case TAG_VERTEX:
                {
                    //debug("received %d", vertex_val);
                    verti i = part.local(vertex_val);
                    assert(!attr[i]);
                    attr[i] = true;
                    queue.push_back(i);
                    ++num_recv;
                    reqs[TAG_VERTEX].Start();
                } break;

            case TAG_PROBE:
                {
                    probe_val[0] += num_send;
                    probe_val[1] += num_recv;
                    //debug("received probe (%d, %d)", probe_val[0], probe_val[1]);
                    if (mpi_rank == 0)  // first process checks for termination
                    {
                        if (probe_val[0] == total_recv)
                        {
                            // Termination detected!
                            assert(probe_val[0] == probe_val[1]);
                            assert(total_send == total_recv);
                            for (int i = 1; i < mpi_size; ++i)
                            {
                                MPI::COMM_WORLD.Send( NULL, 0, MPI_INT,
                                                      i, TAG_TERM );
                            }
                            goto terminated;
                        }
                        else
                        {
                            // Not yet terminated.
                            total_send = probe_val[0];
                            total_recv = probe_val[1];
                            probe_val[0] = 0;
                            probe_val[1] = 0;
                            MPI::COMM_WORLD.Send( probe_val, 2, MPI_INT,
                                                  1, TAG_PROBE );
                            //debug("resent probe");
                        }
                    }
                    else  // mpi_rank > 0
                    {
                        int dest = mpi_rank + 1 == mpi_size ? 0 : mpi_rank + 1;
                        MPI::COMM_WORLD.Send( probe_val, 2, MPI_INT,
                                              dest, TAG_PROBE );
                        //debug("forwarded probe");
                    }
                    reqs[TAG_PROBE].Start();
                } break;

            case TAG_TERM:
                goto terminated;

            default:  // should never get here
                assert(0);
                break;
            }
        }
    }

terminated:
    //debug("terminated!");
    for (int i = 0; i < 3; ++i)
    {
        reqs[i].Cancel();
        reqs[i].Free();
    }
    //debug("return %s", str(part, attr).c_str());
}

ParityGameSolver *MpiRecursiveSolverFactory::create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size )
{
    (void)vertex_map;       // unused
    (void)vertex_map_size;  // unused

    return new MpiRecursiveSolver(game);
}
