#!/usr/bin/env python

# TODO: psi/phi

for i in (10,50,100,500,1000,2000,5000):
    print('#chi' + str(i) + ' := (nu X.(' + 'q&<a>('*(2*i) + '!q&<a>X' + ')'*(2*i) + ')) ==> nu Z.mu Y.(!q&<a>Z)|(q&<a>(q&<a>Y));')
