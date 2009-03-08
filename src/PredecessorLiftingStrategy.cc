#include "PredecessorLiftingStrategy.h"
#include "assert.h"

/* TODO: reimplement queue manually (it doesn't need to be dynamic!) */

PredecessorLiftingStrategy::PredecessorLiftingStrategy(
    const ParityGame &game, bool backward, bool stack )
    : LiftingStrategy(game), stack_(stack)
{
    assert(game.graph().edge_dir() & StaticGraph::EDGE_PREDECESSOR);

    /* Initialize queue */
    verti V = game.graph().V();
    queued_.resize(V, true);
    queue_.resize(V);

    if (backward)
    {
        for (verti v = 0; v < V; ++v) queue_.push_back(V - 1 - v);
    }
    else
    {
        for (verti v = 0; v < V; ++v) queue_.push_back(v);
    }
}

verti PredecessorLiftingStrategy::next(verti prev_vertex, bool prev_lifted)
{
    const StaticGraph &graph = game_.graph();

    if (prev_lifted)
    {
        /* prev_vertex was lifted; add its predecessors to the queue */
        for ( StaticGraph::const_iterator it = graph.pred_begin(prev_vertex);
              it != graph.pred_end(prev_vertex); ++it )
        {
            if (!queued_[*it])
            {
                /* Add predecessor to the queue */
                queued_[*it] = true;
                queue_.push_back(*it);
            }
        }
    }

    if (queue_.empty()) return NO_VERTEX;

    verti res;
    if (stack_)
    {
        res = queue_.back();
        queue_.pop_back();
    }
    else
    {
        res = queue_.front();
        queue_.pop_front();
    }
    queued_[res] = false;
    return res;
}
