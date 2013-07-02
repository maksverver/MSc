#!/usr/bin/env python2

import sys
import re

results = []

def mean(vals):
    return 1.0*sum(vals)/len(vals)

def variance(vals):
    m = mean(vals)
    return mean([ (v - m)**2 for v in vals ])

def stddev(vals):
    return variance(vals)**0.5

def values_for_key(key):
    values = []
    for result in results:
        if key in result:
            values.append(result[key])
    return values

param_pattern = re.compile('##\s*([^\s]*)\s*=\s*(.*[^\s])')
for filename in sys.argv[1:]:
    params = {}
    for line in open(filename , 'rt'):
        m = param_pattern.search(line)
        if m:
            params[m.group(1)] = m.group(2)
    if not params:
        print "Failed to read any parameters from {}!".format(filename)
        sys.exit(1)
    results.append(params)

def int_num(i):
    return '\\num{' + str(int(round(i))) + '}'

def test1():
    for strategy in sorted(set(values_for_key('config.spm.strategy'))):
        line = strategy
        points = []
        for outdegree in sorted(set(values_for_key('config.random.outdegree'))):
            lifting_rate  = []   # lifts/second
            for result in results:
                if (result['config.random.outdegree'] == outdegree and
                    result['config.spm.strategy'] == strategy):
                    lifting_rate.append(int(result['lifts.total'])/float(result['solution.time'].rstrip('s')))
            line = line + ' & ' + int_num(mean(lifting_rate)) + ' & ' + int_num(stddev(lifting_rate))
            points.append((int(outdegree), mean(lifting_rate)))
        #print line + ' \\\\'
        print '\\addplot coordinates {', ' '.join(map(str, points)), '};'
        print '\\addlegendentry{' + strategy + '}'

def format_number(num, grp = 3, sep = ','):
    'Converts an integer to a string with a comma after every three decimals'
    s = str(num)
    i = grp
    while i < len(s):
        s = s[:-i] + sep + s[-i:]
        i += grp + len(sep)
    return s

def test4(what):
    assert what in ('time','lifts_per_second')
    inputs  = []  # triple of vertices, clustersize, seed
    configs = []  # triple of solver, strategy, alternate
    for result in results:
        inputs.append( ( int(result['config.random.vertices']),
                        int(result['config.random.clustersize']),
                        int(result['config.random.seed']) ) )
        configs.append( (result['config.solver'],
                        result['config.spm.strategy'],
                        result['config.spm.alternate'] == 'true') )
    inputs = list(sorted(set(inputs)))
    configs = list(sorted(set(configs),
        key = lambda (solver, strategy, alternate): (alternate,solver,strategy.replace('predecessor','maa')) ))

    for (vertices, clustersize, seed) in inputs:
        if seed == 1:
            print '\\hline'
        print str(vertices) + '/' + str(clustersize) + '/' + str(seed),
        for i,(solver, strategy, alternate) in enumerate(configs):
            if i == 12:
                print '\\\\'
            matched = [ result for result in results if
                            int(result['config.random.vertices'])    == vertices and
                            int(result['config.random.clustersize']) == clustersize and
                            int(result['config.random.seed'])        == seed and
                            result['config.solver']         == solver and
                            result['config.spm.strategy']   == strategy and
                            result['config.spm.alternate']  == ['false','true'][alternate] ]
            assert len(matched) == 1
            record, = matched
            time = float(record['solution.time'].rstrip(' s'))
            if what == 'time':
                if record['solution.result'] == 'success':
                    value = str(time)
                else:
                    value = 't/o'
            else:
                value = "%.3f"%(int(record['lifts.total'])/time/1e6)
                if record['solution.result'] != 'success':
                    value = '*' + value
            print '&', value,
        print '\\\\'

test4('time')
