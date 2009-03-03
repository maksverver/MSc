#include <stdio.h>
#include <aterm_init.h>
#include <mcrl2/pbes/pbes.h>
#include <mcrl2/pbes/parity_game_generator.h>

typedef unsigned int verti;

int main(int argc, char *argv[])
{
    MCRL2_ATERMPP_INIT(argc, argv);

    // Read a PBES from standard input
    mcrl2::pbes_system::pbes<> pbes;
    pbes.load("");

    // Generate min-priority parity game
    mcrl2::pbes_system::parity_game_generator pgg(pbes, true, true);

    // Build the edge list
    verti num_vertices = 1 + *pgg.get_initial_values().rbegin();
    for (verti v = 0; v < num_vertices; ++v)
    {
        std::set<unsigned> deps = pgg.get_dependencies(v);
        for ( std::set<unsigned>::const_iterator it = deps.begin();
              it != deps.end(); ++it )
        {
            verti w = (verti)*it;
            if (w >= num_vertices) num_vertices = w + 1;
            printf("%6d -> %6d\n", v, w);
        }
    }

    // Find vertex properties
    for (verti v = 0; v < num_vertices; ++v)
    {
        bool and_op = pgg.get_operation(v) ==
                      mcrl2::pbes_system::parity_game_generator::PGAME_AND;
        int priority = pgg.get_priority(v);
        printf("%6d: player=%d priority=%d\n", v, and_op, priority);
    }
}
