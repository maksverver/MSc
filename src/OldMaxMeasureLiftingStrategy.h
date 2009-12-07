#ifndef OLD_MAX_MEASURE_LIFTING_STRATEGY_H_INCLUDED
#define OLD_MAX_MEASURE_LIFTING_STRATEGY_H_INCLUDED

#include "SmallProgressMeasures.h"
#include <vector>
#include <set>
#include <utility>

class OldMaxMeasureLiftingStrategy : public LiftingStrategy
{
public:
    OldMaxMeasureLiftingStrategy( const ParityGame &game,
                                  const SmallProgressMeasures &spm );
    ~OldMaxMeasureLiftingStrategy();

    verti next(verti prev_vertex, bool prev_lifted);
    size_t memory_use() const;

protected:
    std::vector<verti> vec(verti v) {
        return std::vector<verti>(spm_.vec(v), spm_.vec(v) + spm_.len(v));
    }

private:
    const SmallProgressMeasures &spm_;
    typedef std::set<std::pair<std::vector<verti>, verti> > queue_t;
    queue_t queue_;
    std::vector<queue_t::iterator> queue_pos_;
};


class OldMaxMeasureLiftingStrategyFactory : public LiftingStrategyFactory
{
public:
    LiftingStrategy *create( const ParityGame &game,
                             const SmallProgressMeasures &spm );
};

#endif /* ndef OLD_PREDECESSOR_LIFTING_STRATEGY_H_INCLUDED */
