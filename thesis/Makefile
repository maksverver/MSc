thesis.dvi: thesis.tex references.bib
	latex thesis
	bibtex thesis
	latex thesis
	latex thesis

all: thesis.dvi

clean:
	rm -f *.log *.aux *.bbl *.blg

distclean: clean
	rm -f thesis.dvi

.PHONY: all clean distclean
