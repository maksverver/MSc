#ifndef LIFTING_STRATEGY_H_INCLUDED
#define LIFTING_STRATEGY_H_INCLUDED

#include "ParityGame.h"
#include <string>

/*! Instances of this class encapsulate vertex lifting strategies to be used
    with the small progress measures parity game solver. */
class LiftingStrategy
{
public:
    /*! Create a lifting strategy for the given game from a string description.
        Returns NULL if the  description could not be interpreted. */
    static LiftingStrategy *create( const ParityGame &game,
                                    const std::string description );

    /*! Construct a strategy for the given parity game. */
    LiftingStrategy(const ParityGame &game)
        : graph_(game.graph()), game_(game) { };

    /*! Destroy the strategy */
    virtual ~LiftingStrategy() { };

    /*! Select the next vertex to lift.

        This method is called repeatedly by the SPM solver; the return value
        indicates which vertex to attempt to lift next. If lifting succeeds,
        the vertex will have a greater progress measure vector assigned to it.
        When no more vertices can be lifted, NO_VERTEX should be returned.

        \param prev_vertex Index of the vertex returned by the previous call
                           (or NO_VERTEX for the first call).
        \param prev_lifted Indicates wheter the vertex could be lifted.
    */
    virtual verti next(verti prev_vertex, bool prev_lifted) = 0;

    /*! Returns an estimation of the peak memory use for this strategy. */
    virtual size_t memory_use() const { return 0; }

protected:
    const StaticGraph &graph_;  /*!< the game graph to work on */
    const ParityGame &game_;    /*!< the parity game to work on */
};

#endif /* ndef LIFTING_STRATEGY_H_INCLUDED */
