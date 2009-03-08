#include "LiftingStrategy.h"
#include "LinearLiftingStrategy.h"
#include "PredecessorLiftingStrategy.h"
#include "FocusListLiftingStrategy.h"
#include <stdlib.h>
#include <strings.h>

LiftingStrategy *LiftingStrategy::create( const ParityGame &game,
                                          const std::string description )
{
    if (description.empty()) return NULL;

    // Split into parts, separated by semicolon characters
    std::vector<std::string> parts;
    std::string::size_type i, j;
    for (i = 0; (j = description.find(':', i)) != std::string::npos; i = j + 1)
    {
        parts.push_back(std::string(description, i, j - i));
    }
    parts.push_back(std::string(description, i, j));

    if (strcasecmp(parts[0].c_str(), "linear") == 0)
    {
        int backward = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        return new LinearLiftingStrategy(game, backward);
    }
    else
    if (strcasecmp(parts[0].c_str(), "predecessor") == 0)
    {
        int backward = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        int stack    = (parts.size() > 2 ? atoi(parts[2].c_str()) : 0);
        return new PredecessorLiftingStrategy(game, backward, stack);
    }
    else
    if ( strcasecmp(parts[0].c_str(), "focuslist") == 0 ||
         strcasecmp(parts[0].c_str(), "focus") == 0 )
    {
        int backward = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        return new FocusListLiftingStrategy(game, backward);
    }
    else
    {
        // No suitable strategy found
        return NULL;
    }
}
