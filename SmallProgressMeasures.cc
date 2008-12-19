#include "SmallProgressMeasures.h"
#include <algorithm>
#include <assert.h>
#include <string.h>

LiftingStatistics::LiftingStatistics(const ParityGame &game)
    : lifts_attempted_(0), lifts_succeeded_(0), vertex_stats_(game.graph().V())
{
}

void LiftingStatistics::record_lift(verti v, bool success)
{
    ++lifts_attempted_;
    ++vertex_stats_[v].first;
    if (success)
    {
        ++lifts_succeeded_;
        ++vertex_stats_[v].second;
    }
}

SmallProgressMeasures::SmallProgressMeasures( const ParityGame &game,
    LiftingStrategy &strategy, LiftingStatistics *stats )
    : ParityGameSolver(game), strategy_(strategy),
      len_(game.d()/2), stats_(stats)
{
    /* Initialize SPM vector bounds */
    M_ = new verti[len_];
    for (int n = 0; n < len_; ++n) M_[n] = game_.cardinality(2*n + 1);

    /* Initialize SPM vector data */
    size_t n = (size_t)len_*game.graph().V();
    spm_ = new verti[n];
    std::fill_n(spm_, n, 0);
}

SmallProgressMeasures::~SmallProgressMeasures()
{
    delete[] spm_;
    delete[] M_;
}

inline int SmallProgressMeasures::vector_cmp(verti v, verti w)
{
    if (is_top(v)) return is_top(w) ? 0 : +1;   /* v is top */
    if (is_top(w)) return -1;                   /* w is top, but v isn't */

    for (int n = 0; n < len(v); ++n)
    {
        if (vec(v)[n] < vec(w)[n]) return -1;
        if (vec(v)[n] > vec(w)[n]) return +1;
    }

    return 0;
}

inline verti SmallProgressMeasures::get_ext_succ(verti v, bool take_max)
{
    const verti *it  = game_.graph().succ_begin(v),
                *end = game_.graph().succ_end(v);

    assert(it != end);  /* assume we have at least one successor */

    verti res = *it++;
    for ( ; it != end; ++it)
    {
        int d = vector_cmp(*it, res);
        if (take_max ? d > 0 : d < 0) res = *it;
    }
    return res;
}

verti SmallProgressMeasures::get_min_succ(verti v)
{
    return get_ext_succ(v, false); 
}

verti SmallProgressMeasures::get_max_succ(verti v)
{
    return get_ext_succ(v, true);
}

bool SmallProgressMeasures::lift(verti v)
{
    if (is_top(v)) return false;

    bool player_even = game_.player(v) == ParityGame::PLAYER_EVEN;
    bool priority_even = game_.priority(v)%2 == 0;

    /* Find relevant successor */
    verti w = player_even ? get_min_succ(v) : get_max_succ(v);

    /* Successor is larger than current node; we must lift it. */
    if (is_top(w))
    {
        set_top(v);
        return true;
    }

    /* See if lifting is required */
    int d = vector_cmp(v, w);

    bool carry;
    if (priority_even)
    {
        if (d >= 0) return false;
        carry = false;
    }
    else /* !priority_even */
    {
        if (d > 0) return false;
        carry = true;
    }

    /* Assign successor */
    for (int n = len(v) - 1; n >= 0; --n)
    {
        vec(v)[n] = vec(w)[n] + carry;
        carry = (vec(v)[n] == M_[n]);
        if (carry) vec(v)[n] = 0;
    }
    if (carry) set_top(v);

    return true;
}

bool SmallProgressMeasures::solve()
{
    verti vertex = NO_VERTEX;
    bool lifted = false;

    while ((vertex = strategy_.next(vertex, lifted)) != NO_VERTEX)
    {
        lifted = lift(vertex);
        if (stats_ != NULL) stats_->record_lift(vertex, lifted);
    }

    return true;
}

ParityGame::Player SmallProgressMeasures::winner(verti v)
{
    return is_top(v) ? ParityGame::PLAYER_ODD : ParityGame::PLAYER_EVEN;
}


#include <stdio.h>  /* debug */

void SmallProgressMeasures::debug_print()
{
    for (verti v = 0; v < game_.graph().V(); ++v)
    {
        printf ( "%6d %c p=%d:", (int)v,
                 game_.player(v) == ParityGame::PLAYER_EVEN ? 'E' : 'O',
                 (int)game_.priority(v) );
        if (is_top(v))
        {
            printf(" T");
        }
        else
        {
            for (int p = 0; p < game_.d(); ++p)
            {
                printf(" %d", p%2 == 0 ? 0 : vec(v)[p/2]);
            }
        }
        printf("\n");
    }

    printf("Verification %s\n", verify_solution() ? "succeeded." : "failed!");
}

bool SmallProgressMeasures::verify_solution()
{
    const StaticGraph &graph = game_.graph();

    for (verti v = 0; v < graph.V(); ++v)
    {
        if (!is_top(v))
        {
            for (int p = 0; p < game_.d(); ++p)
            {
                if (p%2 == 0) continue; /* no even components stored */

                /* Ensure vector values satisfy bounds */
                if (vec(v)[p/2] >= M_[p/2])
                {
                    printf( "%d-th component of SPM vector for vertex %d "
                            "out of bounds!\n", p/2, (int)v );
                    return false;
                }

                if (p > game_.priority(v) && vec(v)[p/2] != 0)
                {
                    printf( "%d-th component of SPM vector for vertex %d "
                            "should be zero!\n", p/2, (int)v );
                    return false;
                }
            }
        }

        bool player_even = game_.player(v) == ParityGame::PLAYER_EVEN;
        bool priority_even = game_.priority(v)%2 == 0;

        bool all_ok = true, one_ok = false;
        for ( StaticGraph::const_iterator it = graph.succ_begin(v);
              it != graph.succ_end(v); ++it )
        {
            int d = vector_cmp(v, *it);
            bool ok = priority_even ? d >= 0 : (d > 0 || is_top(v));
            one_ok = one_ok || ok;
            all_ok = all_ok && ok;
        }

        if (!(player_even ? one_ok : all_ok))
        {
            printf( "order constraint not satisfied for vertex %d with "
                    "priority %d and player %s!\n", v, game_.priority(v),
                    player_even ? "even" : "odd" );
            return false;
        }
    }
    return true;
}
