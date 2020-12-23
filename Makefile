# standard Linux makefile

.PHONY=program test runall clean

DOT_FILES=$(wildcard out/*.dot)
SVG_FILES = $(patsubst %.dot,%.svg,$(DOT_FILES))

INPUT_FILES=$(wildcard src/*.cpp)
OBJ_FILES = $(patsubst src/%.cpp,obj/%.o,$(INPUT_FILES))
HEADERS=$(wildcard src/*.h)

SAMPLE_FILES=$(wildcard samples/*.*)

CFLAGS=-Wall -std=c++11 -fexceptions

#----------------------------------------------
# Test mode ?
#ifeq "$(TEST)" ""
#	TEST=N
#endif
#ifeq ($(TEST),Y)
#	CFLAGS += -DTESTMODE
#endif

#----------------------------------------------
# Generate dot files (for X-cycles)
ifeq "$(GENDOT)" ""
	GENDOT=N
endif
ifeq ($(GENDOT),Y)
	CFLAGS += -DGENERATE_DOT_FILES
endif

#----------------------------------------------
# Build without udgcd
ifeq "$(UDGCD)" ""
	UDGCD=YES
endif

ifeq ($(UDGCD),NO)
	CFLAGS += -DBUILD_WITHOUT_UDGCD
endif



program: sudoku
	@echo "done target $@"

runall: program
	@echo "start solving all samples" > all_samples.log
	@for f in samples/*.sud; do echo "RUNNING $$f"; ./sudoku -c -f $$f; echo "file $$f: success=$$?">>all_samples.log; done

# linking binary
sudoku: $(OBJ_FILES) obj/main.o
	$(CXX) -o sudoku $(OBJ_FILES) obj/main.o  -s
	@echo "done target $@"

test_catch: $(OBJ_FILES) obj/test_catch.o
	$(CXX) -o test_catch $(OBJ_FILES) obj/test_catch.o -DTESTMODE -s
	@echo "done target $@"

# generic compile rule
obj/%.o: src/%.cpp $(HEADERS) Makefile
	$(CXX) $(CFLAGS) -o $@ -c $<

obj/main.o: main.cpp $(HEADERS) Makefile
	$(CXX) $(CFLAGS) -o $@ -c $< -Isrc

obj/test_catch.o: src/test/test_catch.cpp $(HEADERS) Makefile
	$(CXX) $(CFLAGS) -o $@ -c $<

dot: $(SVG_FILES)
	@echo done $<

%.svg: %.dot
	dot -Tsvg $< > $@

dox: html/index.html

html/index.html: $(INPUT_FILES) $(HEADERS) doxyfile Makefile
	doxygen doxyfile


show:
	@echo "OBJ_FILES=$(OBJ_FILES)"
	@echo "HEADERS=$(HEADERS)"

clean:
	-rm obj/*.o
	-rm out/*

cleandoc:
	-rm -R html/*

test: CFLAGS += -DTESTMODE

test: test_catch
	./test_catch -s

help:
	@echo "Available targets:"
	@echo " -test: build & run unit tests"
	@echo " -dox: build doxygen pages"
	@echo " -runall: build  program and run it on all the provided samples"


