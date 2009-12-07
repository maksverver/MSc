#ifndef ATTRACTOR_H_INCLUDED
#define ATTRACTOR_H_INCLUDED

#include "ParityGame.h"

/*! Computes the attractor set of the given vertex set for a specific player,
    and stores it in-place in `vertices'. If `strategy' is not NULL, it is
    updated for all vertices added that are controlled by `player'. */
template<class SetT>
void make_attractor_set( const ParityGame &game, ParityGame::Player player,
                         SetT &vertices, ParityGame::Strategy *strategy );

#include "attractor_impl.h"

#endif /* ndef ATTRACTOR_H_INCLUDED */
