// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef PARITY_GAME_SOLVER
#define PARITY_GAME_SOLVER

#include "ParityGame.h"
#include "Abortable.h"
#include "RefCounted.h"
#include <vector>

/*! Merges a substrategy into a main strategy, overwriting the existing strategy
    for all vertices with indices in vertex_map. */
void merge_strategies( std::vector<verti> &strategy,
                       const std::vector<verti> &substrat,
                       const std::vector<verti> &vertex_map );

/*! Merges two vertex maps, by translating the indices from begin to end using
    old_map, such that new_map[i] == old_map[new_map[i]] or NO_VERTEX if
    new_map[i] >= old_map_size. */
template<class ForwardIterator>
void merge_vertex_maps( ForwardIterator begin, ForwardIterator end,
                        const verti *old_map, verti old_map_size );

/*! Abstract base class for parity game solvers: classes that encapsulate
    algorithms to compute the winning set and optimal strategies in a game. */
class ParityGameSolver : public Abortable, RefCounted
{
public:
    ParityGameSolver(const ParityGame &game)
        : game_(game) { };
    virtual ~ParityGameSolver() { };

    /*! Solve the game and return the strategies for both players. */
    virtual ParityGame::Strategy solve() = 0;

    /*! Returns the parity game for this solver instance. */
    const ParityGame &game() const { return game_; }

protected:
    const ParityGame &game_;           //!< Game being solved
};

/*! Abstract base class for parity game solver factories. */
class ParityGameSolverFactory : public RefCounted
{
public:
    virtual ~ParityGameSolverFactory() { };

    /*! Create a parity game solver for the given game.
        \param vertex_map maps vertex indices from the given subgame to the
            main game. (This allows the SPM solver to correctly collect per-
            vertex lifting statistics even if the game is decomposed first.)
        \param vertex_map_size number of vertices mapped */
    virtual ParityGameSolver *create( const ParityGame &game,
        const verti *vertex_map = NULL, verti vertex_map_size = 0 ) = 0;
};

#include "ParityGameSolver_impl.h"

#endif /* ndef PARITY_GAME_SOLVER */
