#!/usr/bin/env python2

from math import *
from operator import mul
from glob import glob
import sys
import re

param_pattern = re.compile('##\s*([^\s]*)\s*=\s*(.*[^\s])')

def mean(vals):
    if not vals: return 0
    return 1.0*sum(vals)/len(vals)

def variance(vals):
    if not vals: return 0
    m = mean(vals)
    return mean([ (v - m)**2 for v in vals ])

def stddev(vals):
    if not vals: return 0
    return sqrt(variance(vals))

def geom_mean(vals):
    if not vals: return 0
    return reduce(mul, vals) ** (1.0/len(vals))

def geom_stddev(vals):
    if not vals: return 0
    m = geom_mean(vals)
    return exp(sqrt(sum(log(v/m)**2 for v in vals)/len(vals)))

def values_for_key(key):
    values = []
    for result in results:
        if key in result:
            values.append(result[key])
    return values

def parse_file(filename):
    params = { 'filename': filename }
    for line in open(filename , 'rt'):
        m = param_pattern.search(line)
        if m:
            params[m.group(1)] = m.group(2)
    if not params:
        print "Failed to read any parameters from {}!".format(filename)
        sys.exit(1)
    return params

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

def print_test4_table(print_time):
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

    # This prints summarized output with solvers on the rows, and inputs on the columns.
    # For each solver/input combo, there are multiple seeds. We report how many cases
    # are solved, and the geometric mean time of the **solved** cases.
    last_alternate = False
    for (solver, strategy, alternate) in configs:

        # Print a fancy table row header:
        names = { 'linear':         'Linear',
                  'predecessor':    'Predecessor',
                  'minmeasure':     'Min. Measure',
                  'maxmeasure':     'Max. Measure',
                  'maxstep':        'Max. Step' }

        variants = { 'linear': "FA",
                     'predecessor': "QS",
                     'minmeasure': "QSH",
                     'maxmeasure': "QSH",
                     'maxstep':    "SQH" }

        strat_id,strat_variant = strategy.split(':')

        print names[strat_id].ljust(12), '&', variants[strat_id][int(strat_variant)], " I"[solver == "spm2"], " T"[alternate],

        for (clustersize, vertices) in sorted(set((clustersize, vertices) for (vertices, clustersize,seed) in inputs)):
            matched = [ result for result in results if
                            int(result['config.random.vertices'])    == vertices and
                            int(result['config.random.clustersize']) == clustersize and
                            result['config.solver']         == solver and
                            result['config.spm.strategy']   == strategy and
                            result['config.spm.alternate']  == ['false','true'][alternate] ]
            times = [ float(record['solution.time'].rstrip('s')) for record in matched
                        if record['solution.result'] == 'success' ]
            lifts = [ float(record['lifts.total']) for record in matched
                        if record['solution.result'] == 'success' ]
            if print_time:
                print '&', len(times), '&', ('$%.3f$'%mean(times)),
                #print '&', len(times), '&', ('$%.3f \pm %.3f$'%(mean(times), stddev(times))),
            else:
                print '&', len(times), '&', ('$%.3f$'%(geom_mean(lifts)/1e6)),
                #print '&', len(times), '&', ('$%.3f \pm %.3f$'%(mean(lifts)/1e6, stddev(lifts)/1e6)),
        print '\\\\'

    return

    # This prints table with inputs on the rows, and solvers on the columns:
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

            if 0:
                # Report time
                if record['solution.result'] == 'success':
                    value = str(time)
                else:
                    value = 't/o'
                print '&', value,

        print '\\\\'

def print_spm_spgip_plots(param, case, ns, suffix = '', a=False):
    configs = []  # triple of solver, strategy, alternate
    configs = [ '-llinear:0', '-Lpredecessor:0', '-Lminmeasure:0', '-Lmaxmeasure:0', '-Lmaxstep:0' ]
    for config in configs:
        print '\\addplot coordinates {',
        for n in ns:
            file, = glob('output-spm-spgip-shuffled/pgsolver-' + case + str(n) + suffix + config + ("-a" if a else "") + '.o*')
            params = parse_file(file)
            if params['solution.result'] == 'success':
                print '(%d,%s)'%(n, params[param].rstrip(' s')),
        print '};%',config

    print '\\legend{%s}' % ','.join('{%s}'%c for c in configs)


def print_spm_spgip_graphs(label, param, a=False):
    caption_start = ['SPM lifting strategy performance', 'Two-sided SPM performance'][a]
    id = label.lower().replace(' ','_')
    if a: id += '_a'

    print """\
\\begin{figure}
\\centering
\\makebox[0pt][c]{% 
\\begin{minipage}{10cm}   % phi
\\begin{tikzpicture}
\\begin{semilogyaxis}[xlabel=$\\phi_n$,ylabel={"""+label+"""},legend pos=south east]"""
    print_spm_spgip_plots(param,'phi',(2,3,4,5,6,7,8),a=a)
    print """\
\\legend{{lin},{pred},{min},{max},{step}}
\\end{semilogyaxis}
\\end{tikzpicture}
\\end{minipage}

\\begin{minipage}{10cm}   % chi
\\begin{tikzpicture}
\\begin{loglogaxis}[xlabel=$\\phi'_n$,ylabel={"""+label+"""},legend pos=south east]
"""
    print_spm_spgip_plots(param,'chi',(10,50,100,500,1000,2000,5000),a=a)
    print """\
\\legend{{lin},{pred},{min},{max},{step}} 
\\end{loglogaxis}
\\end{tikzpicture}
\\end{minipage}
}%
\caption{"""+caption_start+""" on decision procedures (in """+label.lower()+""")}
\label{spm_spgip_graphs_deciproc_"""+id+"""}
\\end{figure}

\\begin{figure}
\\centering

\\makebox[0pt][c]{%
\\begin{minipage}{10cm}   % elevator-fair
\\begin{tikzpicture}
\\begin{loglogaxis}[xlabel=$G_n$,ylabel={"""+label+"""},legend pos=south east]"""
    print_spm_spgip_plots(param,'elevator',(3,4,5,6,7,8), '-fair',a=a)
    print """\
\\legend{{lin},{pred},{min},{max},{step}} 
\\end{loglogaxis}
\\end{tikzpicture}
\\end{minipage}

\\begin{minipage}{10cm}   % elevator-unfair
\\begin{tikzpicture}
\\begin{loglogaxis}[xlabel=$G'_n$,ylabel={"""+label+"""},legend pos=south east]"""
    print_spm_spgip_plots(param,'elevator',(3,4,5,6,7,8), '-unfair',a=a)
    print """\
\\legend{{lin},{pred},{min},{max},{step}} 
\\end{loglogaxis}
\\end{tikzpicture}
\\end{minipage}
}%
\caption{"""+caption_start+""" on elevator verification (in """+label.lower()+""")}
\label{spm_spgip_graphs_elevator_"""+id+"""}
\\end{figure}"""

def print_spm_spgip_twosided_table():
    print """\
\\begin{table}
\\begin{tabular}{ c|c|c|c|c|}
\\hline 
& Lin. & Pred. & Max.Meas. & Max.Step \\tabularnewline
\\hline"""
    last_case = None
    for (latex,case,n,suffix) in [
        ('\\phi','phi',5,''),
        ('\\phi','phi',6,''),
        ('\\phi','phi',7,''),
        ('\\phi','phi',8,''),
        ('\\phi\'','chi', 100,''),
        ('\\phi\'','chi', 500,''),
        ('\\phi\'','chi',1000,''),
        ('\\phi\'','chi',2000,''),

        ('G', 'elevator', 5, '-fair'),
        ('G', 'elevator', 6, '-fair'),
        ('G', 'elevator', 7, '-fair'),
        ('G', 'elevator', 8, '-fair'),

        ('G\'', 'elevator', 5, '-unfair'),
        ('G\'', 'elevator', 6, '-unfair'),
        ('G\'', 'elevator', 7, '-unfair'),
        ('G\'', 'elevator', 8, '-unfair') ]:

        if case+suffix != last_case:
            last_case = case+suffix
            print '\\hline'

        print "$" + latex + "_{" + str(n) + "}$",
        for config in [ '-llinear:0', '-Lpredecessor:0', '-Lmaxmeasure:0', '-Lmaxstep:0' ]:
            print '&',
            file1, = glob('output-spm-spgip-shuffled/pgsolver-' + case + str(n) + suffix + config +       '.o*')
            file2, = glob('output-spm-spgip-shuffled/pgsolver-' + case + str(n) + suffix + config + "-a" + '.o*')
            params1 = parse_file(file1)
            params2 = parse_file(file2)
            if params1['solution.result'] == 'success' and \
               params2['solution.result'] == 'success':
                print ('%.3f' % (float(params1['lifts.total'])/float(params2['lifts.total']))),
            else:
                print 'n/a',
        print "\\tabularnewline"
        print "\\hline"
    print """\
\\end{tabular}
\\caption{Ratio of lifting attempts performed to solve using two-sided SPM compared to regular SPM}
\\label{tab:spm_spgip_twosided_lift_ratio}
\\end{table}"""

def print_spm_spgip_barchart(case, title):

    config_ids = [ 'Linear',     'Predecessor',     'MaxMeasure',     'MaxStep'     ]
    configs    = [ '-llinear:0', '-Lpredecessor:0', '-Lmaxmeasure:0', '-Lmaxstep:0' ]

    results_oneway = []
    results_twoway = []

    for config in configs:
        file1, = glob('output-spm-spgip-shuffled/pgsolver-' + case + config +        '.o*')
        file2, = glob('output-spm-spgip-shuffled/pgsolver-' + case + config + "-a" + '.o*')
        params1 = parse_file(file1)
        params2 = parse_file(file2)
        assert params1['solution.result'] == 'success'
        assert params2['solution.result'] == 'success'
        results_oneway.append(params1['lifts.total'])
        results_twoway.append(params2['lifts.total'])

    print """\
\\begin{tikzpicture}
\\begin{semilogxaxis}[
        xbar,
        xlabel={Lifting attempts},
        symbolic y coords={""" + ','.join(config_ids) + """},
        title={"""+title+"""},
        ytick=data,
        y dir=reverse,
        legend pos=south east,
]
\\addplot [draw=blue,pattern=horizontal lines dark blue] coordinates {"""

    for (config_id, results) in zip(config_ids, results_twoway):
        print '(%s,%s)'%(results,config_id),

    print """};
\\addplot [draw=black,pattern=horizontal lines light blue] coordinates {"""

    for (config_id, results) in zip(config_ids, results_oneway):
        print '(%s,%s)'%(results,config_id),

    print """};
\\legend{Two-sided,One-sided}
\\end{semilogxaxis}
\\end{tikzpicture}"""


def print_spm_spgip_barcharts():

    print """\
\\begin{figure}
\\centering
\\makebox[0pt][c]{% 
\\begin{minipage}{10cm}   % phi"""
    print_spm_spgip_barchart("phi8", "$\phi_8$")
    print """\
\\end{minipage}

\\begin{minipage}{10cm}   % chi"""
    print_spm_spgip_barchart("chi500", "$\phi'_{500}$")
    print """\
\\end{minipage}
}%
\\caption{One-way vs two-way solving on decision procedures cases}
\\label{spm_spgip_charts_deciproc}
\\end{figure}

\\begin{figure}
\\centering
\\makebox[0pt][c]{% 
\\begin{minipage}{10cm}   % elevator-fair"""
    print_spm_spgip_barchart("elevator8-fair", "$G_8$")
    print """\
\\end{minipage}

\\begin{minipage}{10cm}   % elevator-unfair"""
    print_spm_spgip_barchart("elevator6-unfair", "$G'_6$")
    print """\
\\end{minipage}
}%
\\caption{One-way vs two-way solving on elevator verification cases}
\\label{spm_spgip_charts_elevator}
\\end{figure}"""


stdout = sys.stdout
if False:
    with open('results-spm-spgip-lifts.tex', 'wt') as sys.stdout:
        print_spm_spgip_graphs('Lifting attempts', 'lifts.total', a=False)
    with open('results-spm-spgip-time.tex', 'wt') as sys.stdout:
        print_spm_spgip_graphs('Time in seconds', 'solution.time', a=False)
    with open('results-spm-spgip-lifts-a.tex', 'wt') as sys.stdout:
        print_spm_spgip_graphs('Lifting attempts', 'lifts.total', a=True)
    with open('results-spm-spgip-time-a.tex', 'wt') as sys.stdout:
        print_spm_spgip_graphs('Time in seconds', 'solution.time', a=True)

    results = map(parse_file, glob('output-tests4b/random-*.o*'))
    with open('results-tests4-lifts-table.tex', 'wt') as sys.stdout:
        print_test4_table(print_time = False)
    with open('results-tests4-time-table.tex', 'wt') as sys.stdout:
        print_test4_table(print_time = True)

with open('results-spm-spgip-twoway-charts.tex', 'wt') as sys.stdout:
    print_spm_spgip_barcharts()


#with open('results-spm-spgip-twosided-table.tex', 'wt') as sys.stdout:
#    print_spm_spgip_graphs('Time in seconds', 'solution.time', a=True)
#sys.stdout = stdout

# print_spm_spgip_twosided_table()
#print_spm_spgip_twosided_barchart()

# TODO: compare different flavors (0/1/2) (how?)
