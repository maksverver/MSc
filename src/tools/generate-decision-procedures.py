#!/usr/bin/env python

# Generates the linear time mu-calculus formulas for decision procedure
# benchmarks described in "Solving Parity Games in Practice", which can be
# turned into parity games with mlsolver.

for n in range(2,8):
	psi = ""
	for m in range(n):
		psi += ["mu","nu"][m%2] + " X" + str(m + 1) + "."
	for m in range(n):
		if m > 0: psi += " & "
		psi += "(q" + str(m + 1) + " | <a>(X" + str(m + 1)
	psi += "))"*n
	print("#psi" + str(n) + " := " + psi + ";")
	print("#phi" + str(n) + " := #psi" + str(n) + " | !#psi" + str(n) + ";")

for n in [10,50,100,500,1000,2000]:
	phi_prime = "(nu X." +  "(q&<a>"*(2*n) + "(!q&<a>X)" + ")"*(2*n) + ")" \
	          + " ==> nu Z.mu Y.(!q&<a>Z)|(q&<a>(q&<a>Y))"
	# N.B. relabeled from phi' to chi:
	print("#chi" + str(n) + " := " + phi_prime + ";")
