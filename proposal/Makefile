proposal.dvi: proposal.tex references.bib
	latex proposal
	bibtex proposal
	latex proposal
	latex proposal

all: proposal.dvi

clean:
	rm -f *.log *.aux *.bbl *.blg

distclean: clean
	rm -f proposal.dvi

.PHONY: all clean distclean
