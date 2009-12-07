#ifndef RECURSIVE_SOLVER_H_INCLUDED
#define RECURSIVE_SOLVER_H_INCLUDED

#include "ParityGameSolver.h"

class RecursiveSolver : public ParityGameSolver
{
public:
    RecursiveSolver(const ParityGame &game);
    ~RecursiveSolver();

    // pure virtual methods declared by ParityGameSolver:
    ParityGame::Strategy solve();
    size_t memory_use();

private:
    //! Solves a subgame with a known minimum priority:
    ParityGame::Strategy solve(const ParityGame &game, int min_prio);
};

#endif /* ndef RECURSIVE_SOLVER_H_INCLUDED */
