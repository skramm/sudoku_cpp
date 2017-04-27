# standard Linux makefile


DOT_FILES=$(wildcard out/*.dot)
SVG_FILES = $(patsubst %.dot,%.svg,$(DOT_FILES))

INPUT_FILES=grid.cpp x_cycles.cpp main.cpp
OBJ_FILES = $(patsubst %.cpp,obj/%.o,$(INPUT_FILES))
HEADERS=$(wildcard *.h)

CFLAGS=-Wall -std=c++11 -fexceptions

#----------------------------------------------
# Test mode ?
ifeq "$(TEST)" ""
	TEST=N
endif
ifeq ($(TEST),Y)
	CFLAGS += -DTESTMODE
endif


# linking binary
sudoku: $(OBJ_FILES)
	$(CXX) -o sudoku obj/grid.o obj/x_cycles.o obj/main.o  -s

# generic compile rule
obj/%.o: %.cpp $(HEADERS) Makefile
	$(CXX) $(CFLAGS) -o $@ -c $<

dot: $(SVG_FILES)
	@echo done $<

%.svg: %.dot
	dot -Tsvg $< > $@

dox: html/index.html

html/index.html: $(INPUT_FILES)
	doxygen doxyfile


show:
	@echo "OBJ_FILES=$(OBJ_FILES)"
	@echo "HEADERS=$(HEADERS)"

clean:
	-rm obj/*.o

test: sudoku
	./sudoku
