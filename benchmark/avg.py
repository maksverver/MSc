#!/usr/bin/env python

import sys
values = [ float(line) for line in sys.stdin ]
print round(sum(values)/len(values),3)
