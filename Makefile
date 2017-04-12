# WIP

INPUT_FILES=grid.cpp x_cycles.cpp main.cpp

obj/%.o: %.cpp
	$(CXX) -Wall -std=c++11 -fexceptions -O2  -c grid.cpp -o obj/grid.o

# generic compile rule
obj/%.o: %.cpp $(HEADERS) makefile
	$(CXX) -o $@ -c $<

# linking
%: obj/%.o
	$(CXX) -o $@ -s $<  $(LDFLAGS)

dox: html/index.html

html/index.html: $(INPUT_FILES)
	doxygen doxyfile
