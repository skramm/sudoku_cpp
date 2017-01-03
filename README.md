# sudoku_cpp

Just a small weekend side project...

A sudoku C++ solver

* I/O is text only
 * Input: a file name, holding the puzzle as a 9x9 character grid (see in samples folder for examples)
 * Ouput: the solved grid. Or more if verbose flag (-v) is given.
* Uses basic strategies, and one more advanced one (XY-Wings).
* Requirements: a C++11 compiler
* Licence: GPL v3
* Author: S. Kramm

** Instructions **

1. Build with `$ ./build.sh`
2. Run with `$ sudoku samples/grid2.sud`

Options:

* `-v`: verbose, will print out the mains steps and algorithms used
* `-s`: will print the steps (elminating a candidate in a cell)
