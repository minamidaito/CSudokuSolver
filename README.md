# Introduction
I found the modest
but non-trivial programming challenge it offered enticing. Sudoku is solvable
algorithmically and so, unlike crosswords, admits a full computer solution.
Therefore I bring you a  Sudoku solver. It's written in C and
I have commented it liberally.

# Description
This program is designed to work for all Sudoku puzzles. If you find one that
it can't handle then please contact me.

Some Sudoku solving techniques apparently require the solution to be unique.
This program has no such restriction. Indeed, sample number four is the
following somewhat ambiguous puzzle ("0" means "unknown"):

      0 0 0  0 0 0  0 0 0
      0 0 0  0 0 0  0 0 0
      0 0 0  0 0 0  0 0 0

      0 0 0  0 0 0  0 0 0
      0 0 0  0 0 0  0 0 0
      0 0 0  0 0 0  0 0 0

      0 0 0  0 0 0  0 0 0
      0 0 0  0 0 0  0 0 0
      0 0 0  0 0 0  0 0 0

The solution the Nihilist Sudoku solver produces for the above grid is as
follows:

      1 2 3  4 5 6  7 8 9
      4 5 6  7 8 9  1 2 3
      7 8 9  1 2 3  4 5 6

      2 1 4  3 6 5  8 9 7
      3 6 5  8 9 7  2 1 4
      8 9 7  2 1 4  3 6 5

      5 3 1  6 4 2  9 7 8
      6 4 2  9 7 8  5 3 1
      9 7 8  5 3 1  6 4 2

# How it Works
The code works using a candidate list of for each of the 81 entries,
as follows:

   1. Make all numbers 1 to 9 a candidate for each entry;
   2. Reduce the candidate list to a single entry for each entry given in the
      puzzle posed;
   3. Apply the Sudoku logic rules to eliminate all the candidates we can from
      other, initially unknown, entries;
   4. At this stage, lesser Sudoku solvers give up if they haven't got a full
      solution. However, the Nihilist brand Sudoku solver then uses recursion
      and backtracking in a spirit similar to a fiendishly difficuly
      programming language called Prolog. The code takes a still-ambiguous
      entry and fixes its value to one of the remaining candidates. It then
      tries to solve the puzzle with this entry fixed. If the puzzle cannot
      be solved it tries the next candidate. One of the candidates will work,
      provided the puzzle has a solution at all. On attempting to solve the
      puzzle with a hypothesized value for an as-yet-undetermined value the
      code may have to hypothesize a fixed value for another entry; this may
      continue to progressively deeper levels. This is the recursive step.

On my modestly-powered laptop in the early 2000s, a typical run time is less than 0.1 second. The
most time-consuming puzzle, sample9.txt, takes less than 1/8th of a second.

The code does not contain any platform-dependent calls and, as such, should
build on any platform that has a C compiler. I have tested it on Linux as well
as Windows.

# Update 9 July 2005
Someone has kindly pointed out that the following puzzle causes the Sudoku Solver code to run slowly:

      0 0 1  0 0 0  0 0 0
      0 0 0  0 0 0  2 0 0
      0 0 0  0 0 0  3 0 0

      0 0 0  0 0 0  0 4 0
      0 0 0  0 0 0  0 5 0
      0 0 0  0 0 0  0 0 6

      0 0 0  0 7 0  0 0 0
      0 0 0  8 0 0  0 0 0
      0 0 0  0 0 0  9 0 0

This puzzle causes the program to continue processing for some minutes. I
looked into the behavior and determined that it was not caused by a logic bug.
This particular puzzle is simply tough on my algorithm. I found two
workarounds. The first is to move the last three columns of the puzzle so that
they become the first three columns. Then the code solves the puzzle
immediately. The second workaround is to modify the educated guess part of the
code so that it tries the highest-value remaining candidate first rather than
the lowest.
