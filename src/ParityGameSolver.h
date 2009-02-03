#ifndef PARITY_GAME_SOLVER
#define PARITY_GAME_SOLVER

#include "ParityGame.h"

class ParityGameSolver
{
public:
    ParityGameSolver(const ParityGame &game) : game_(game) { };
    virtual ~ParityGameSolver() { };

    /* Solve the game. */
    virtual bool solve() = 0;

    /* After the game has been solved, this function returns the winner of
       the parity game when starting from vertex i. */
    virtual ParityGame::Player winner(verti v) = 0;

protected:
    const ParityGame &game_;
};

#endif /* ndef PARITY_GAME_SOLVER */
