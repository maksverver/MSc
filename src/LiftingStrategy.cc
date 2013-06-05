// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "LiftingStrategy.h"
#include "LinearLiftingStrategy.h"
#include "PredecessorLiftingStrategy.h"
#include "FocusListLiftingStrategy.h"
#include "MaxMeasureLiftingStrategy.h"
#include "OldMaxMeasureLiftingStrategy.h"
#include "LinPredLiftingStrategy.h"

#include <stdlib.h>

#include "compatibility.h"
#define strcasecmp compat_strcasecmp

LiftingStrategyFactory::~LiftingStrategyFactory()
{
}

const char *LiftingStrategyFactory::usage()
{
    return
"linear:backward:alternate\n"
"   Use a linear lifting strategy (aka swiping).\n"
"   - backward: if 1, scan vertices backward (default: 0)\n"
"   - alternate: if 1, switches direction between forward and backward \n"
"     whenever the end of the list is reached (default: 0)\n"
"\n"
"predecessor:backward:stack\n"
"   Use a predecessor lifting strategy (aka worklist).\n"
"   - backward: if 1, the queue is initialized in reverse (default: 0)\n"
"   - stack: if 1, removes elements from the end of the queue instead \n"
"            of the beginning (default: 0)\n"
"\n"
"focuslist:backward:alternate:max_size:lift_ratio\n"
"   Use swiping + focus list lifting strategy.\n"
"   - backward: see 'linear' (default: 0)\n"
"   - alternate: see 'linear' (default: 0)\n"
"   - max_size: the maximum size of the focus list, either as an absolute size\n"
"     greater than 1, or as a ratio (between 0 and 1) of the total number of\n"
"     vertices (default: 0.1)\n"
"   - lift_ratio: the maximum number of lifting attempts performed on the\n"
"     focus list before switching back to swiping, as a ratio of the maximum\n"
"     focus list size (default: 10.0)\n"
"\n"
"maxmeasure:backward:order\n"
"   Maximum measure propagation; a variant of the predecessor lifting strategy\n"
"   that prefers to lift vertices with higher progress measures.\n"
"   - backward: see 'predecessor' (default: 0)\n"
"   - order: tie-breaking lifting order: 0 (queue-like), 1 (stack-like)\n"
"            or 2 (heap order) (default: 2)\n"
"\n"
"oldmaxmeasure\n"
"   Old implementation of max. measure lifting strategy.\n"
"   Included for regression testing purposes only.\n"
"\n"
"linpred\n"
"   Obsolete (roughly equivalent to predecessor:0:0).\n";
}

LiftingStrategyFactory *
    LiftingStrategyFactory::create(const std::string &description)
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

    if ( strcasecmp(parts[0].c_str(), "linear") == 0 ||
         strcasecmp(parts[0].c_str(), "lin") == 0 )
    {
        bool backward  = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        bool alternate = (parts.size() > 2 ? atoi(parts[2].c_str()) : 0);
        return new LinearLiftingStrategyFactory(backward, alternate);
    }
    else
    if ( strcasecmp(parts[0].c_str(), "predecessor") == 0 ||
         strcasecmp(parts[0].c_str(), "pred") == 0 )
    {
        bool backward = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        bool stack    = (parts.size() > 2 ? atoi(parts[2].c_str()) : 0);
        return new PredecessorLiftingStrategyFactory(backward, stack);
    }
    else
    if ( strcasecmp(parts[0].c_str(), "focuslist") == 0 ||
         strcasecmp(parts[0].c_str(), "focus") == 0 )
    {
        bool backward     = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        bool alternate    = (parts.size() > 2 ? atoi(parts[2].c_str()) : 0);
        double max_size   = (parts.size() > 3 ? atof(parts[3].c_str()) : 0);
        double lift_ratio = (parts.size() > 4 ? atof(parts[4].c_str()) : 0);
        return new FocusListLiftingStrategyFactory(
            backward, alternate, max_size, lift_ratio );
    }
    else
    if (strcasecmp(parts[0].c_str(), "maxmeasure") == 0)
    {
        bool backward = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        int  order    = (parts.size() > 2 ? atoi(parts[2].c_str()) : 2);
        return new MaxMeasureLiftingStrategyFactory(
            backward, (MaxMeasureLiftingStrategy2::Order)order );
    }
    else
    if (strcasecmp(parts[0].c_str(), "oldmaxmeasure") == 0)
    {
        return new OldMaxMeasureLiftingStrategyFactory();
    }
    else
    if (strcasecmp(parts[0].c_str(), "linpred") == 0)
    {
        return new LinPredLiftingStrategyFactory();
    }
    else
    {
        // No suitable strategy found
        return NULL;
    }
}
