// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef LIFTING_STRATEGY_H_INCLUDED
#define LIFTING_STRATEGY_H_INCLUDED

#include "ParityGame.h"
#include "RefCounted.h"
#include <string>

class SmallProgressMeasures;

/*! \defgroup LiftingStrategies

    Lifting strategies for the Small Progress Measures solving algorithm.
*/

/*! \ingroup LiftingStrategies

    Instances of this class encapsulate vertex lifting strategies to be used
    with the small progress measures parity game solver.
*/
class LiftingStrategy
{
public:

    /*! Construct a strategy for the given parity game. */
    LiftingStrategy(const ParityGame &game)
        : graph_(game.graph()), game_(game) { };

    /*! Destroy the strategy */
    virtual ~LiftingStrategy() { };

    /*! Record that the given vertex was lifted: */
    virtual void lifted(verti vertex) = 0;

    /*! Select the next vertex to lift. This method is called repeatedly by the
        SPM solver until it returns NO_VERTEX to indicate the solution is
        complete.

        \see lifted(verti vertex)
    */
    virtual verti next() = 0;

protected:
    const StaticGraph &graph_;          //!< the game graph to work on
    const ParityGame &game_;            //!< the parity game to work on
};

class LiftingStrategy2
{
public:

    LiftingStrategy2(const ParityGame &game)
        : graph_(game.graph()), game_(game) { };

    virtual ~LiftingStrategy2() { };

    virtual void push(verti vertex) = 0;
    virtual void bump(verti vertex) = 0;
    virtual verti pop() = 0;

protected:
    const StaticGraph &graph_;          //!< the game graph to work on
    const ParityGame &game_;            //!< the parity game to work on
};

/*! \ingroup LiftingStrategies
    Abstract base class for lifting strategy factories. */
class LiftingStrategyFactory : public RefCounted
{
public:
    virtual ~LiftingStrategyFactory();

    /*! Returns pre-formatted plain-text documentation of the description
        strings accepted by create(), intended to be shown to the user.
        \see create(const std::string &description) */
    static const char *usage();

    /*! Creates a lifting strategy factory from a string description.
    \returns A factory object or NULL if the description could not be parsed.
    \see usage() for a description of available format strings. */
    static LiftingStrategyFactory *create(const std::string &description);

    /*! Create a lifting strategy for the given game, to be used by the given
        Small Progress Measures solver. */
    virtual LiftingStrategy *create( const ParityGame &game,
                                     const SmallProgressMeasures &spm ) = 0;

    virtual LiftingStrategy2 *create2( const ParityGame &game,
                                       const SmallProgressMeasures &spm ) { return 0; }
};


#endif /* ndef LIFTING_STRATEGY_H_INCLUDED */
