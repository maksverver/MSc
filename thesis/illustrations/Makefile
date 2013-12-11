EPS=	introduction-example-solved.eps \
	introduction-example.eps \
	zielonka-steps.eps \
	binary-heap-updates.eps

all: $(EPS)

clean:
	rm -f $(EPS)

%.eps: %.svg
	inkscape --export-eps=$@ $<
