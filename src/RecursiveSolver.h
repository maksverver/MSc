#ifndef RECURSIVE_SOLVER_H_INCLUDED
#define RECURSIVE_SOLVER_H_INCLUDED

#include "ParityGameSolver.h"

class RecursiveSolver : public ParityGameSolver
{
public:
    RecursiveSolver(const ParityGame &game);
    ~RecursiveSolver();

    ParityGame::Strategy solve();

private:
    //! Solves a subgame with a known minimum priority:
    ParityGame::Strategy solve(const ParityGame &game, int min_prio);
};

class RecursiveSolverFactory : public ParityGameSolverFactory
{
    ParityGameSolver *create( const ParityGame &game,
        const verti *vertex_map, verti vertex_map_size );
};

#endif /* ndef RECURSIVE_SOLVER_H_INCLUDED */
