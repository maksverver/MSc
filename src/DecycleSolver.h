// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DECYCLE_SOLVER_H_INCLUDED
#define DECYCLE_SOLVER_H_INCLUDED

#include "SmallProgressMeasures.h"
#include "DenseSet.h"
#include "Logger.h"
#include "SCC.h"
#include <deque>
#include <string>
#include <vector>

/*! A solver that first removes i-dominated cycles controlled by player p
    for all values of i and p where i%2 == p, then calls another, general
    solver to solve the remaining subgame. */
class DecycleSolver : public ParityGameSolver, public virtual Logger
{
public:
    DecycleSolver( const ParityGame &game, ParityGameSolverFactory &pgsf,
                   const verti *vertex_map, verti vertex_map_size );
    ~DecycleSolver();

    ParityGame::Strategy solve();

private:
    // SCC callback
    int operator()(const verti *vertices, size_t num_vertices);
    friend class SCC<DecycleSolver>;

    size_t my_memory_use();

protected:
    ParityGameSolverFactory &pgsf_;       //!< Solver factory to use
    const verti             *vmap_;       //!< Current vertex map
    const verti             vmap_size_;   //!< Size of vertex map

    // Used in the SCC callback:
    int                    prio_;      //!< current selected priority
    std::vector<verti>     mapping_;   //!< current priority induced vertex set
    StaticGraph            graph_;     //!< current priority induced subgraph
    std::deque<verti>      winning_;   //!< current winning vertices
    ParityGame::Strategy   strategy_;  //!< current winning strategy
};

class DecycleSolverFactory : public ParityGameSolverFactory
{
public:
    DecycleSolverFactory(ParityGameSolverFactory &pgsf)
        : pgsf_(pgsf) { pgsf_.ref(); }
    ~DecycleSolverFactory() { pgsf_.deref(); }

    ParityGameSolver *create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size );

protected:
    ParityGameSolverFactory &pgsf_;     //!< Factory used to create subsolvers
};

#endif /* ndef DECYCLE_SOLVER_H_INCLUDED */