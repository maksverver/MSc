// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "AsyncMpiAttractorAlgorithm.h"

extern int mpi_rank, mpi_size;

AsyncMpiAttractorImpl::AsyncMpiAttractorImpl( const VertexPartition &vpart,
        const GamePartition &part, ParityGame::Player player,
        std::vector<char> &attr, std::deque<verti> &queue,
        ParityGame::Strategy &strategy )
    : vpart_(vpart), part(part), player(player), attr(attr), queue(queue),
      strategy_(strategy), num_send(0), num_recv(0)
{
    info("Constructed AsyncMpiAttractorImpl.");

    reqs[TAG_VERTEX] = MPI::COMM_WORLD.Recv_init(
        &vertex_val, 1, MPI_INT, MPI::ANY_SOURCE, TAG_VERTEX );

    reqs[TAG_PROBE] = MPI::COMM_WORLD.Recv_init(
        &probe_val[0], 2, MPI_INT, MPI::ANY_SOURCE, TAG_PROBE ),

    reqs[TAG_TERM] = MPI::COMM_WORLD.Recv_init(
        NULL, 0, MPI_INT, 0, TAG_TERM );

    MPI::Prequest::Startall(3, reqs);
}

AsyncMpiAttractorImpl::~AsyncMpiAttractorImpl()
{
    for (int i = 0; i < 3; ++i)
    {
        reqs[i].Cancel();
        reqs[i].Free();
    }
}

void AsyncMpiAttractorImpl::solve(bool quick_start)
{
    const StaticGraph &graph = part.game().graph();

    // Uses Friedemann Mattern's four-counter method for termination detection.
    // All processes keep track of the number of sent and received messages.
    // When idle, the first process sends a probe that is circulated to other
    // processes when they are idle, accumulating the total number of messages
    // sent and received. This has to be done twice in order to confirm global
    // termination, at which point the first process sends a termination signal
    // to the other processes.

    bool send_probe = (mpi_rank == 0);
    int total_send = 0, total_recv = 0;

    // When quick starting, processes are aware of each other's initial vertex
    // sets. If not, then we must first transmit the contents of the initial
    // set to the relevant other processes.
    if (!quick_start)
    {
        // N.B. queue.size() cannot be calculated inside the loop, because
        //      notify_others may push new, non-local vertices in the queue!
        size_t n = queue.size();
        for (size_t i = 0; i < n; ++i) notify_others(queue[i]);
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

                // Skip vertices already in the attractor set:
                if (attr[v]) continue;

                // Skip vertices not assigned to this worker process:
                if (vpart_(part.global(v)) != mpi_rank) continue;

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
                notify_others(v);

            skip_v:
                continue;
            }
        }

        // Idle: wait for a message to respond to.
        //debug("idle");
        if (send_probe)
        {
            if (mpi_size < 2)
            {
                //debug("single process exiting without a probe");
                return;
            }
            int probe[2] = { 0, 0 };
            MPI::COMM_WORLD.Send(probe, 2, MPI_INT, 1, TAG_PROBE);
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
                            //debug("termination detected")
                            return;
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
                //debug("termination message received");
                return;

            default:  // should never get here
                assert(0);
                break;
            }
        }
    }
    // should never get here!
    assert(0);
}

void AsyncMpiAttractorImpl::notify_others(verti i)
{
    const StaticGraph &graph = part.game().graph();
    verti v = part.global(i);
    std::vector<bool> recipients(mpi_size);
    for ( StaticGraph::const_iterator it = graph.pred_begin(i);
            it != graph.pred_end(i); ++it )
    {
        recipients[vpart_(part.global(*it))] = true;
    }
    for ( StaticGraph::const_iterator it = graph.succ_begin(i);
            it != graph.succ_end(i); ++it )
    {
        recipients[vpart_(part.global(*it))] = true;
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
    while (reqs[TAG_VERTEX].Test())
    {
        //debug("received %d", req_val);
        i = part.local(vertex_val);
        assert(!attr[i]);
        attr[i] = true;
        queue.push_back(i);
        ++num_recv;
        reqs[TAG_VERTEX].Start();
    }
}
