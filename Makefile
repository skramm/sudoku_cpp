# standard Linux makefile

.PHONY=program

DOT_FILES=$(wildcard out/*.dot)
SVG_FILES = $(patsubst %.dot,%.svg,$(DOT_FILES))

INPUT_FILES=grid.cpp x_cycles.cpp algorithms.cpp main.cpp test_catch.cpp
OBJ_FILES = $(patsubst %.cpp,obj/%.o,$(INPUT_FILES))
HEADERS=$(wildcard *.h)

SAMPLE_FILES=$(wildcard samples/*.*)

CFLAGS=-Wall -std=c++11 -fexceptions

#----------------------------------------------
# Test mode ?
ifeq "$(TEST)" ""
	TEST=N
endif
ifeq ($(TEST),Y)
	CFLAGS += -DTESTMODE
endif

program: sudoku
	@echo "done target $@"

# linking binary
sudoku: $(OBJ_FILES)
	$(CXX) -o sudoku obj/algorithms.o obj/grid.o obj/x_cycles.o obj/main.o  -s
	@echo "done target $@"

test_catch: $(OBJ_FILES)
	$(CXX) -o test_catch obj/algorithms.o obj/grid.o obj/x_cycles.o obj/test_catch.o  -s
	@echo "done target $@"

# generic compile rule
obj/%.o: %.cpp $(HEADERS) Makefile
	$(CXX) $(CFLAGS) -o $@ -c $<

dot: $(SVG_FILES)
	@echo done $<

%.svg: %.dot
	dot -Tsvg $< > $@

dox: html/index.html

html/index.html: $(INPUT_FILES) $(HEADERS) doxyfile
	doxygen doxyfile


show:
	@echo "OBJ_FILES=$(OBJ_FILES)"
	@echo "HEADERS=$(HEADERS)"

clean:
	-rm obj/*.o

cleandoc:
	-rm -R html/*

test: test_catch
	./test_catch

