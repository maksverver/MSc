#include "GamePartition.h"

GamePartition::GamePartition( const ParityGame &old_game,
                              const std::vector<verti> &intern )
{
    // We assume `intern' is sorted and therefore `internal' will
    // be sorted too. This makes it easier to create subpartitions later.
    assert(is_sorted(intern.begin(), intern.end(), std::less<verti>()));

    std::vector<verti> verts(intern.begin(), intern.end());

    // Find vertices incident to internal vertices
    for (std::vector<verti>::const_iterator it = intern.begin();
            it != intern.end(); ++it)
    {
        // Add predecessors of internal vertices
        for (StaticGraph::const_iterator jt = old_game.graph().pred_begin(*it);
                jt != old_game.graph().pred_end(*it); ++jt) verts.push_back(*jt);

        // Add successors of internal vertices
        for (StaticGraph::const_iterator jt = old_game.graph().succ_begin(*it);
                jt != old_game.graph().succ_end(*it); ++jt) verts.push_back(*jt);
    }

    // Make vertex set unique
    std::sort(verts.begin(), verts.end());
    verts.erase(std::unique(verts.begin(), verts.end()), verts.end());

    // Create game
    game_.make_subgame(old_game, verts.begin(), verts.end());

    // Create vertex index maps
    global_ = verts;
    for (verti v = 0; v < (verti)global_.size(); ++v)
    {
        local_[global_[v]] = v;
    }
    internal_ = intern;
    for ( std::vector<verti>::iterator it = internal_.begin();
            it != internal_.end(); ++it )
    {
        *it = local_[*it];
    }

    // FIXME?  We are currently storing successor/predecessor edges for
    //         external vertices too, but these are never used!
}

GamePartition::GamePartition( const GamePartition &part,
                              const std::vector<verti> &verts )
{
    std::set_intersection( verts.begin(), verts.end(),
                        part.internal_.begin(), part.internal_.end(),
                        std::back_inserter(internal_) );

    std::vector<verti> new_verts;
    // FIXME: we need to remove vertices that do not have any edges incident
    //        to the internal vertex set, but this is a bit ugly:
    {
        const StaticGraph &g = part.game_.graph();
        HASH_SET(verti) used(internal_.begin(), internal_.end());
        for (std::vector<verti>::const_iterator it = verts.begin();
                it != verts.end(); ++it)
        {
            verti v = *it;
            bool found = used.find(v) != used.end();
            for (StaticGraph::const_iterator it = g.succ_begin(v);
                    !found && it != g.succ_end(v); ++it)
            {
                if (used.find(*it) != used.end()) found = true;
            }
            for (StaticGraph::const_iterator it = g.pred_begin(v);
                    !found && it != g.pred_end(v); ++it)
            {
                if (used.find(*it) != used.end()) found = true;
            }
            if (found) new_verts.push_back(v);
        }
    }

    game_.make_subgame(part.game_, new_verts.begin(), new_verts.end());
    global_.resize(new_verts.size());
    for (verti i = 0; i < (verti)global_.size(); ++i)
    {
        global_[i] = part.global_[new_verts[i]];
        local_[global_[i]] = i;
    }

    // Map internal vertices to new local indices.
    // FIXME: is there a more efficient way to do this?
    for ( std::vector<verti>::iterator it = internal_.begin();
            it != internal_.end(); ++it )
    {
        *it = local_[part.global_[*it]];
    }

    // DEBUG: check consistency of vertex index mapping
    /*
    for (verti i = 0; i < (verti)global_.size(); ++i)
    {
        assert(local_[global_[i]] == i);
    }
    for (HASH_MAP(verti, verti)::const_iterator it = local_.begin();
            it != local_.end(); ++it)
    {
        assert(global_[it->second] == it->first);
    }
    */
}
