// Copyright (c) 2007, 2009 University of Twente
// Copyright (c) 2007, 2009 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef SMALL_PROGRESS_MEASURES_H_INCLUDED
#define SMALL_PROGRESS_MEASURES_H_INCLUDED

#include "ParityGameSolver.h"
#include "LiftingStrategy.h"
#include "Logger.h"
#include <vector>
#include <utility>

/*! Object used to collect statistics when solving using the SPM algorithm */
class LiftingStatistics
{
public:
    /*! Construct a statistics object for the given game. */
    LiftingStatistics(const ParityGame &game);

#if 0
    /*! Merge statistics from a given object into this object, using the given
        vertex mapping to map vertex indices (vertex v in `other' has index
        mapping[v] in this object). */
    void merge(const LiftingStatistics &other, const verti *mapping = NULL);
#endif

    long long lifts_attempted() const { return lifts_attempted_; }
    long long lifts_succeeded() const { return lifts_succeeded_; }
    long long lifts_attempted(verti v) const { return vertex_stats_[v].first; }
    long long lifts_succeeded(verti v) const { return vertex_stats_[v].second; }

private:
    void record_lift(verti v, bool success);
    friend class SmallProgressMeasures;

private:
    long long lifts_attempted_, lifts_succeeded_;
    std::vector<std::pair<long long, long long> > vertex_stats_;
};

/* Implementation of Jurdziński's Small Progress Measures algorithm. */
class SmallProgressMeasures : public Abortable, public virtual Logger
{
public:
    SmallProgressMeasures( const ParityGame &game, ParityGame::Player player,
        LiftingStatistics *stats = 0, const verti *vertex_map = 0,
        verti vertex_map_size = 0 );

    ~SmallProgressMeasures();

    /*! Solves the current game for one player using the given lifting strategy
        and returns whether the game was completely solved (in particular, the
        game is not solved if the solver is aborted). */
    bool solve(LiftingStrategy &ls);

    /*! Solves part of the game by doing attemping at most `max_attempts' lifts
        using the given lifting strategy. Returns how many lifting attempts
        were actually performed, which will be less than `max_attempts' when
        the game is solved. */
    long long solve_part(LiftingStrategy &ls, long long max_attempts);

    /*! Takes an initialized strategy vector and updates it for the current
        player. The result is valid only if the game is completely solved. */
    void get_strategy(ParityGame::Strategy &strat) const;

    /*! Returns the winning set for the given player by assigning the vertices
        in the set to the given output iterator. If the game is not completely
        solved yet, then this returns a subset of the winning set. */
    template<class OutputIterator>
    void get_winning_set(ParityGame::Player player, OutputIterator result);

    /*! Return peak memory use (excludes lifting strategy!) */
    size_t memory_use();

    /*! Sets the given vertex's progress measure to top, if it isn't already,
        and returns whether it changed: */
    inline bool lift_to_top(verti v);

    /*! For debugging: print current state to stdout */
    void debug_print(bool verify = true);

    /*! For debugging: verify that the current state describes a valid SPM */
    bool verify_solution();

protected:
    /*! Attempt to lift a vertex (and return whether this succeeded). */
    bool lift(verti v);

    /*! Return the SPM vector for vertex `v`.
        This array contains only the components with odd (for Even) or even
        (for Odd) indices of the vector (since the reset is fixed at zero). */
    verti *vec(verti v) { return &spm_[(size_t)len_*v]; }
    const verti *vec(verti v) const { return &spm_[(size_t)len_*v]; }

    /*! Return the number of odd priorities less than or equal to the
        priority of v. This is the length of the SPM vector for `v`. */
    int len(verti v) const { return (game_.priority(v) + 1 + p_)/2; }

    /*! Return whether the SPM vector for vertex `v` has top value. */
    bool is_top(verti v) const { return vec(v)[0] == NO_VERTEX; }

    /*! Set the SPM vector for vertex `v` to top value. */
    inline void set_top(verti v);

private:
    SmallProgressMeasures(const SmallProgressMeasures &);
    SmallProgressMeasures &operator=(const SmallProgressMeasures &);

private:
    /*! Compares the first `N` elements of the SPM vectors for the given
        vertices and returns -1, 0 or 1 to indicate that v is smaller, equal to,
        r larger than w (respectively). */
    inline int vector_cmp(verti v, verti w, int N) const;

    /*! Returns the minimum or maximum successor for vertex `v`,
        depending on whether take_max is false or true (respectively). */
    inline verti get_ext_succ(verti v, bool take_max) const;

    /*! Returns the minimum successor for vertex `v`. */
    verti get_min_succ(verti v) const;

    /*! Returns the maximum successor for vertex `v`. */
    verti get_max_succ(verti v) const;

    // Allow selected lifting strategies to access the SPM internals:
    friend class PredecessorLiftingStrategy;
    friend class MaxMeasureLiftingStrategy;
    friend class OldMaxMeasureLiftingStrategy;

protected:
    const ParityGame &game_;        //!< the game being solved
    const int p_;                   //!< the player to solve for
    LiftingStatistics *stats_;      //!< statistics object to record lifts
    const verti *vmap_;             //!< active vertex map (if any)
    verti vmap_size_;               //!< size of vertex map
    int len_;                       //!< length of SPM vectors
    verti *M_;                      //!< bounds on the SPM vector components
    verti *spm_;                    //!< array storing the SPM vector data
};


/*! A parity game solver based on Marcin Jurdzinski's small progress measures
    algorithm, with pluggable lifting heuristics.

    For each node, we need to keep an SPM vector of length game->d().
    However, since all components with even indices (zero-based) are fixed at 0,
    we only store values for the odd indices.
*/
class SmallProgressMeasuresSolver
    : public ParityGameSolver, public virtual Logger
{
public:
    SmallProgressMeasuresSolver( const ParityGame &game,
                                 LiftingStrategyFactory &lsf,
                                 bool alternate = false,
                                 LiftingStatistics *stats = 0,
                                 const verti *vertex_map = 0,
                                 verti vertex_map_size = 0 );
    ~SmallProgressMeasuresSolver();

    ParityGame::Strategy solve();

    /*! Solves the game by applying Jurdziński's proposed algorithm that solves
        the game for one player only, and then solves a subgame with the
        remaining vertices. This algorithm is most efficient when the original
        game is easier to solve than its dual. */
    ParityGame::Strategy solve_normal();

    /*! Solves the game using Friedmann's alternate strategy. This allocates
        solving algorithms for both the normal game and its dual at once, and
        alternates working on each, exchanging information about solved vertices
        in the process. */
    ParityGame::Strategy solve_alternate();

    /*! Preprocess the game so that vertices with loops either have the loop
        removed, or have all other edges removed. In the latter case, the vertex
        is necessarily won by the player corresponding with its parity.

        This preprocessing operation speeds up solving with small progress
        measures considerably, though it is superseded by the DecycleSolver
        which does more general preprocessing. */
    static void preprocess_game(ParityGame &game);

private:
    SmallProgressMeasuresSolver(const SmallProgressMeasuresSolver&);
    SmallProgressMeasuresSolver &operator=(const SmallProgressMeasuresSolver&);

protected:
    LiftingStrategyFactory &lsf_;   //!< factory used to create lifting strategy
    bool alternate_;                //!< whether to use the alternate algorithm
    LiftingStatistics *stats_;      //!< object to record lifting statistics
    const verti *vmap_;             //!< current vertex map
    const verti vmap_size_;         //!< size of vertex map
};


class SmallProgressMeasuresSolverFactory : public ParityGameSolverFactory
{
public:
    SmallProgressMeasuresSolverFactory( LiftingStrategyFactory &lsf,
        bool alt = false, LiftingStatistics *stats = 0 )
            : lsf_(lsf), alt_(alt), stats_(stats) { };

    ParityGameSolver *create( const ParityGame &game,
                              const verti *vertex_map,
                              verti vertex_map_size );

private:
    LiftingStrategyFactory  &lsf_;
    bool                    alt_;
    LiftingStatistics       *stats_;
};

#include "SmallProgressMeasures_impl.h"

#endif /* ndef SMALL_PROGRESS_MEASURES_H_INCLUDED */
