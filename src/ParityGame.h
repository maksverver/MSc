#ifndef PARITY_GAME_H_INCLUDED
#define PARITY_GAME_H_INCLUDED

#include "Graph.h"
#include <iostream>
#include <vector>

/*! Information stored for each vertex of a parity game:
    - the player to move (either PLAYER_EVEN or PLAYER_ODD)
    - the priority of the vertex  between 0 and `d` (exclusive).
    \sa ParityGame::Player
*/
struct ParityGameVertex
{
    unsigned char player, priority;
};

/*! A parity game extends a directed graph by assigning a player
    (Even or Odd) and an integer priority to every vertex.
    Priorities are between 0 and `d` (exclusive). */
class ParityGame
{
public:
    /*! The two players in a parity game (Even and Odd) */
    enum Player { PLAYER_NONE = -1, PLAYER_EVEN = 0, PLAYER_ODD = 1 };

    /*! Construct an empty parity game */
    ParityGame();

    /*! Destroy a parity game */
    ~ParityGame();

    /*! Generate a random parity game, with vertices assigned uniformly at
        random to players, and priority assigned uniformly between 0 and d-1.
        \sa StaticGraph::make_random()
    */
    void make_random( verti V, unsigned out_deg,
                      StaticGraph::EdgeDirection edge_dir, int d );

    /*! Read a game description in PGSolver format. */
    void read_pgsolver(std::istream &is, StaticGraph::EdgeDirection edge_dir);

    /*! Read a game description from an mCRL2 PBES. */
    void read_pbes( const std::string &file_path,
                    StaticGraph::EdgeDirection edge_dir );

    /*! Returns the memory used to store the parity game.
        This includes memory used by the graph! */
    size_t memory_use() const;

    /*! Return the priority limit */
    int d() const { return d_; }

    /*! Return the game graph */
    const StaticGraph &graph() const { return graph_; }

    /*! Return the priority associated with vertex v */
    int priority(verti v) const { return vertex_[v].priority; }

    /*! Return the player associated with vertex v */
    Player player(verti v) const { return (Player)vertex_[v].player; }

    /*! Return the number of vertices with priority `p`.
        `p` must be between 0 and `d` (exclusive). */
    verti cardinality(int p) const { return cardinality_[p]; }

protected:
    /*! Re-allocate memory to store information on V vertices with priorities
        between 0 and `d` (exclusive). */
    void reset(verti V, int d);

private:
    explicit ParityGame(const ParityGame &game);
    ParityGame &operator=(const ParityGame &game);

private:
    int d_;                 /*!< priority limit */
    StaticGraph graph_;     /*!< game graph */

    /*! Assignment of players and priorities to vertices (size graph_.V()) */
    ParityGameVertex *vertex_;

    /*! Cardinality counts for priorities.
        cardinality_[p] is equal to the number of vertices with priority p. */
    verti *cardinality_;
};

#endif /* ndef PARITY_GAME_H_INCLUDED */
