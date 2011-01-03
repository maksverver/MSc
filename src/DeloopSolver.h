// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DELOOP_SOLVER_H_INCLUDED
#define DELOOP_SOLVER_H_INCLUDED

#include "SmallProgressMeasures.h"
#include "DenseSet.h"
#include "Logger.h"
#include "SCC.h"
#include <deque>
#include <string>
#include <vector>

/*! A solver that takes a game preprocessed with SmallProgressMeasures::pre-
    process_game(), identifies vertices with loops (which are won by the player
    corresponding with the parity of their priority) and removes their attractor
    sets from the game to obtain a loop-less reduced game that is then solved
    with a new solver.

    Similar to the DecycleSolver, except being less general yet faster. */
class DeloopSolver : public ParityGameSolver, public virtual Logger
{
public:
    DeloopSolver( const ParityGame &game, ParityGameSolverFactory &pgsf,
                  const verti *vertex_map, verti vertex_map_size );
    ~DeloopSolver();

    ParityGame::Strategy solve();

private:
    // SCC callback
    int operator()(const verti *vertices, size_t num_vertices);
    friend class SCC<DeloopSolver>;

    size_t my_memory_use();

protected:
    ParityGameSolverFactory &pgsf_;       //!< Solver factory to use
    const verti             *vmap_;       //!< Current vertex map
    const verti             vmap_size_;   //!< Size of vertex map
};

class DeloopSolverFactory : public ParityGameSolverFactory
{
public:
    DeloopSolverFactory(ParityGameSolverFactory &pgsf)
        : pgsf_(pgsf) { pgsf_.ref(); }
    ~DeloopSolverFactory() { pgsf_.deref(); }

    ParityGameSolver *create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size );

protected:
    ParityGameSolverFactory &pgsf_;     //!< Factory used to create subsolvers
};

#endif /* ndef DELOOP_SOLVER_H_INCLUDED */
