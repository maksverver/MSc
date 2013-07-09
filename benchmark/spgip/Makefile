MLSOLVER=../../../mlsolver/bin/mlsolver
GENDECIPROC=../../src/tools/generate-decision-procedures.py
GENELEVATOR=../../../pgsolver/bin/elevatorverification

DECIPROC=\
	phi2.pgsolver \
	phi3.pgsolver \
	phi4.pgsolver \
	phi5.pgsolver \
	phi6.pgsolver \
	phi7.pgsolver \
	phi8.pgsolver \
	chi10.pgsolver \
	chi50.pgsolver \
	chi100.pgsolver \
	chi500.pgsolver \
	chi1000.pgsolver \
	chi2000.pgsolver \
	chi5000.pgsolver

ELEVATOR_FAIR=\
	elevator3-fair.pgsolver \
	elevator4-fair.pgsolver \
	elevator5-fair.pgsolver \
	elevator6-fair.pgsolver \
	elevator7-fair.pgsolver \
	elevator8-fair.pgsolver

ELEVATOR_UNFAIR=\
	elevator3-unfair.pgsolver \
	elevator4-unfair.pgsolver \
	elevator5-unfair.pgsolver \
	elevator6-unfair.pgsolver \
	elevator7-unfair.pgsolver \
        elevator8-unfair.pgsolver

ELEVATOR=$(ELEVATOR_FAIR) $(ELEVATOR_UNFAIR)

GAMES=$(DECIPROC) $(ELEVATOR)

all: $(GAMES)
deciproc.env: $(GENDECIPROC); $(GENDECIPROC) >$@
$(DECIPROC): %.pgsolver: deciproc.env; $(MLSOLVER) -env $< -val lmmc \#$(shell basename $@ .pgsolver) --printgame >$@
$(ELEVATOR_FAIR):   %.pgsolver: $(GENELEVATOR); $(GENELEVATOR)    $(shell echo $@ | tr -dc '[0-9]') >$@
$(ELEVATOR_UNFAIR): %.pgsolver: $(GENELEVATOR); $(GENELEVATOR) -u $(shell echo $@ | tr -dc '[0-9]') >$@
clean: ; rm -f deciproc.env $(GAMES)
.PHONY: all clean
