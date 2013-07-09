#!/usr/bin/env python

# Decision procedure formulae from Solving Parity Games in Practice.
# (See Makefile how to turn these into parity games using MLSolver.)

for n in range(2,8+1):
    print( '#psi' + str(n) + ' := ' +
           '.'.join( [ 'mn'[i%2] + 'u X' + str(i + 1) for i in range(n) ] +
                     [ ' & '.join( '(q' + str(i + 1) + ' | <a>(X' + str(i + 1)
                                   for i in range(n) ) ] ) +
           ')'*(2*n) + ';' )
    print('#phi' + str(n) + ' := #psi' + str(n) + ' | !#psi' + str(n) + ';')

for i in (10,50,100,500,1000,2000,5000):
    print('#chi' + str(i) + ' := (nu X.(' + 'q&<a>('*(2*i) + '!q&<a>X' + ')'*(2*i) + ')) ==> nu Z.mu Y.(!q&<a>Z)|(q&<a>(q&<a>Y));')
