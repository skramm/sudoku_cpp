# sudoku_cpp

Just a small weekend side project...

A sudoku C++ solver

* I/O is text only
 * Input: a file name, holding the puzzle as a 9x9 character grid (see in samples folder for examples)
 * Ouput: the solved grid. Or more if verbose flag (-v) is given.
* Uses basic strategies, and one more advanced one (XY-Wings).
* Requirements: a C++11 compiler
* Licence: GPL v3
* Author: Sebastien Kramm (firstname.lastname@univ-rouen.fr)  
In case of any bug, thanks for mailing (or open an issue).

** Instructions **

1. Build with `$ ./build.sh`
2. Run with (for example) `$ ./sudoku samples/grid2.sud`

** Usage and options **

`$ sudoku [options] file`

Options:
* `-v`: verbose, will print out the mains steps and algorithms used. You'd better redirect in a file with that one, lots of output.
* `-s`: will print the steps (elminating a candidate in a cell)

Return values:

* 0: success (solved)
* 1 : failed to solve puzzle
* 2 : failed to read input file
* 3 : invalid puzzle
