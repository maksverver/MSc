// Copyright (c) 2009-2013 University of Twente
// Copyright (c) 2009-2013 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2013 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2013 Eindhoven University of Technology
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
"linear:alternate\n"
"   Use a linear lifting strategy (aka swiping).\n"
"   - alternate: if 1, switches direction between forward and backward \n"
"     whenever the end of the list is reached (default: 0)\n"
"\n"
"predecessor:stack\n"
"   Use a predecessor lifting strategy (aka worklist).\n"
"   - stack: if 1, removes elements from the end of the queue instead \n"
"            of the beginning (default: 0)\n"
"\n"
"focuslist:alternate:max_size:lift_ratio\n"
"   Use swiping + focus list lifting strategy.\n"
"   - alternate: see 'linear' (default: 0)\n"
"   - max_size: the maximum size of the focus list, either as an absolute size\n"
"     greater than 1, or as a ratio (between 0 and 1) of the total number of\n"
"     vertices (default: 0.1)\n"
"   - lift_ratio: the maximum number of lifting attempts performed on the\n"
"     focus list before switching back to swiping, as a ratio of the maximum\n"
"     focus list size (default: 10.0)\n"
"\n"
"maxmeasure:order\n"
"   Maximum measure propagation; a variant of the predecessor lifting strategy\n"
"   that prefers to lift vertices with higher progress measures.\n"
"   - order: tie-breaking lifting order: 0 (queue-like), 1 (stack-like)\n"
"            or 2 (heap order) (default: 2)\n"
"\n"
"maxstep:order\n"
"   Maximum step variant of maximum measure propagation.\n"
"   - order: see 'maxmeasure'\n"
"\n"
"minmeasure:order\n"
"   Minimum measure propagation.\n"
"   - order: see 'maxmeasure'\n"
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
        bool alternate = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        return new LinearLiftingStrategyFactory(alternate);
    }
    else
    if ( strcasecmp(parts[0].c_str(), "predecessor") == 0 ||
         strcasecmp(parts[0].c_str(), "pred") == 0 )
    {
        bool stack    = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        return new PredecessorLiftingStrategyFactory(stack);
    }
    else
    if ( strcasecmp(parts[0].c_str(), "focuslist") == 0 ||
         strcasecmp(parts[0].c_str(), "focus") == 0 )
    {
        bool alternate    = (parts.size() > 1 ? atoi(parts[1].c_str()) : 0);
        double max_size   = (parts.size() > 2 ? atof(parts[2].c_str()) : 0);
        double lift_ratio = (parts.size() > 3 ? atof(parts[3].c_str()) : 0);
        return new FocusListLiftingStrategyFactory(
            alternate, max_size, lift_ratio );
    }
    else
    if (strcasecmp(parts[0].c_str(), "maxmeasure") == 0)
    {
        int order = (parts.size() > 1 ? atoi(parts[1].c_str()) : 2);
        return new MaxMeasureLiftingStrategyFactory(
            (MaxMeasureLiftingStrategy2::Order)order,
            MaxMeasureLiftingStrategy2::MAX_VALUE);
    }
    else
    if (strcasecmp(parts[0].c_str(), "maxstep") == 0)
    {
        int order = (parts.size() > 1 ? atoi(parts[1].c_str()) : 2);
        return new MaxMeasureLiftingStrategyFactory(
            (MaxMeasureLiftingStrategy2::Order)order, 
            MaxMeasureLiftingStrategy2::MAX_STEP );
    }
    else
    if (strcasecmp(parts[0].c_str(), "minmeasure") == 0)
    {
        int order = (parts.size() > 1 ? atoi(parts[1].c_str()) : 2);
        return new MaxMeasureLiftingStrategyFactory(
            (MaxMeasureLiftingStrategy2::Order)order, 
            MaxMeasureLiftingStrategy2::MIN_VALUE );
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
