# WIP



DOT_FILES=$(wildcard out/*.dot)
SVG_FILES = $(patsubst %.dot,%.svg,$(DOT_FILES))

INPUT_FILES=grid.cpp x_cycles.cpp main.cpp


obj/%.o: %.cpp
	$(CXX) -Wall -std=c++11 -fexceptions -O2  -c grid.cpp -o obj/grid.o

# generic compile rule
obj/%.o: %.cpp $(HEADERS) makefile
	$(CXX) -o $@ -c $<

# linking
%: obj/%.o
	$(CXX) -o $@ -s $<  $(LDFLAGS)

dot: $(SVG_FILES)
	@echo done $<

%.svg:%.dot
	dot -Tsvg $< > $@

dox: html/index.html

html/index.html: $(INPUT_FILES)
	doxygen doxyfile
