# sudoku_cpp

A sudoku C++ solver

* I/O is text only
 * Input: a file name, holding the puzzle as a 9x9 character grid (see in samples folder for examples)
 * Ouput: the solved grid. Or more if verbose flag (-v) is given.
* Uses basic strategies, and one more advanced one (XY-Wings).
* Requirements: a C++11 compiler (see below for dependencies).
* Licence: GPL v3
* Author: Sebastien Kramm (firstname.lastname@univ-rouen.fr)

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

`$ sudoku [options] file`

Options:

* `-s`: will print the steps (eliminating a candidate in a cell)
* `-v`: verbose, will print out the mains steps and algorithms used. You'd better redirect in a file with that one, lots of output. Implies option "-s".

Return values:

* 0: success (solved)
* 1 : failed to solve puzzle
* 2 : failed to read input file
* 3 : invalid puzzle

## 3 - Motivation

Started ous as just a small weekend side project...

I found out that solving a Sudoku puzzle is tougher than what I thought. Several techniques can be used,
some very advanced (and difficult to use with only pen and paper!).
For these, the best resource is [Andrew C. Stuart's' website](http://www.sudokuwiki.org/sudoku.htm), from whom I took a lot of inspiration.
Unfortunately, although it give a very good explanation of the "algorithm", it doesn't actually describe that algorithm.
So just consider this as some challenge to analyse and implement (some of) the techniques described on Andrew's site.
