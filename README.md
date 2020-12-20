# sudoku_cpp

A sudoku C++ solver

* I/O is text only
 * Input: a file name, holding the puzzle as a 9x9 character grid (see in samples folder for examples)
 * Ouput: the solved grid. Or more if verbose flag (-v) is given.
* Uses basic strategies, and one more advanced one (XY-Wings).
* Requirements: a C++11 compiler (see below for dependencies).
* Licence: GPL v3
* Author: Sebastien Kramm (firstname.lastname@univ-rouen.fr)
* Home page: [github.com/skramm/sudoku_cpp](https://github.com/skramm/sudoku_cpp)

In case of any bug, thanks for mailing (or open an issue).

## 1 - Instructions

1. Build with `$ make`
2. Run with (for example) `$ ./sudoku samples/grid2.sud`

### dependencies

The "X cycles" algorithm relies on another code of mine: **udgcd** (UnDirected Graph Cycle Detection),
available [here on Github](https://github.com/skramm/udgcd). This library itself depends on BGL
(Boost Graph Library).
If you don't have, or want to build without it, just build with the following command-line:
```
$ make UDGCD=NO
```
The "X cycle" algorithm will be disabled in that case.

For testing purposes, also uses [Catch](https://github.com/philsquared/Catch/), useful only if you want to contribute.


The program comes with several samples. If you want to run them all, type:
```
$ make runall
```

## 2 - Usage and options

`$ sudoku [options] -f file`
`$ sudoku [options] grid`

### 2.1 - Options:

The following switches are available.
Must be separated, no grouping.

* `-l` or `-lx`: activate logging (see below).
* `-v`: verbose, will print out the mains steps and algorithms used.
You'd better redirect in a file with that one, lots of output. Implies option "-l2".
* `-s`: will save the grid to a file name `current.sud` and to a timestamped file (`current_YYYYMMDD_HHMM.sud`).
This is useful when entering grid from command-line, so you can get back to it.

### 2.2 - Logging

The "steps" can be either removing a candidate in a cell, that has a set of candidates, or assigning a value to a cell.
The latter is done automatically when there is only one candidate left.

Two logging levels are available:
* With `-l` or `-l1`, only the steps where a value is assigned to a cell are printed
* With `-l2`, all the steps are printed
* With `-l3`, the algorithm used is printed

### 2.3 Return values (see `$ ./sudoku`):
```
 0: success (solved puzzle)
 1: unable to read given filename (missing or format error)
 2: missing filename after -f
 3: invalid grid given (must be 81 characters, only digits or '.')
 4: invalid grid
 5: unable to solve
```

## 3 - Motivation

Started out as just a small weekend side project...

I found out that solving a Sudoku puzzle is tougher than what I thought.
Several techniques can be used, some very advanced (and difficult to use with only pen and paper!).
For these, the best resource is [Andrew C. Stuart's' website](http://www.sudokuwiki.org/sudoku.htm), from whom I took a lot of inspiration.
Unfortunately, although it gives a very good explanation of the "algorithm", it doesn't actually describe that algorithm.
So just consider this as some challenge to analyse and implement (some of) the techniques described on Andrew's site.

## 4 - history

* 20201115:
 * changed usage: you can now enter grid directly from command-line.
 * new option to save the grid
