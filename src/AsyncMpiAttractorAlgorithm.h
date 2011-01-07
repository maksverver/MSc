// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ASYNC_MPI_ATTRACTOR_ALGORITHM_H_INCLUDED
#define ASYNC_MPI_ATTRACTOR_ALGORITHM_H_INCLUDED

#include "MpiAttractorAlgorithm.h"
#include "Logger.h"

/*! Attractor set computation implementation that runs asynchronously; i.e. all
   worker processes send and receive vertices to be added to the set while they
   are running independent breadth-first search over their local vertex set.
   This is should reduce latency. */
class AsyncMpiAttractorImpl : public Logger
{
public:
    //! tags used to identify different types of messages exchanged through MPI.
    enum MpiTags { TAG_VERTEX, TAG_PROBE, TAG_TERM };

    AsyncMpiAttractorImpl( const VertexPartition &vpart,
                           const GamePartition &part, ParityGame::Player player,
                           std::vector<char> &attr, std::deque<verti> &queue,
                           ParityGame::Strategy &strategy );

    ~AsyncMpiAttractorImpl();

private:
    friend class AsyncMpiAttractorAlgorithm;

    void solve(bool quick_start);

    /*! Helper function for make_attractor_set() that transmits `v' to relevant
        other processes, and then receives any pending vertices from other
        processes, which are then added to `queue' and `attr'. When messages
        are sent or received, `num_send' and `num_recv' are incremented. */
    void notify_others( verti v );

private:
    // TODO: document these + add trailing newlines for consistency
    const VertexPartition       &vpart_;
    const GamePartition         &part;
    const ParityGame::Player    player;
    std::vector<char>           &attr;
    std::deque<verti>           &queue;
    ParityGame::Strategy        &strategy_;
    int                         num_send;
    int                         num_recv;
    verti                       vertex_val;
    int                         probe_val[2];
    MPI::Prequest               reqs[3];
};

class AsyncMpiAttractorAlgorithm : public MpiAttractorAlgorithm
{
    void make_attractor_set( const VertexPartition &vpart,
        const GamePartition &part, ParityGame::Player player,
        std::vector<char> &attr, std::deque<verti> &queue,
        bool quick_start, ParityGame::Strategy &strategy )
    {
        /* Logger::debug( "enter make_attractor_set(%s, %d, %d) in %s",
            part.debug_str(attr).c_str(), (int)player, (int)quick_start,
            part.debug_str().c_str() ); */
        AsyncMpiAttractorImpl impl(vpart, part, player, attr, queue, strategy);
        impl.solve(quick_start);
        // Logger::debug("return -> %s", part.debug_str(attr).c_str());
    }
};

#endif // ndef ASYNC_MPI_ATTRACTOR_ALGORITHM_H_INCLUDED
