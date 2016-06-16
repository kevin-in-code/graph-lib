GraphLib
--------

This repository provides code that may be run to verify published experimental results.

Executable programs included:
    Main_TCS_2015
    CountMaximalCliques

CountMaximalCliques can apply clique enumeration to a graph loaded from a file.
The graph file must be in one of several common text formats.  Example graphs are provided:
    programs/CountMaximalCliques/graph.am.csv,
    programs/CountMaximalCliques/graph.al.csv, and
    programs/CountMaximalCliques/graph.dimacs.txt.


How to build with Visual Studio 2015
------------------------------------

1. Install premake5.

2. On the command line, run:

    premake5 vs2015

3. Open the vs2015 project and build as Win64 Release.


How to build with GCC
---------------------

1. make

This should compile without any gcc/g++ errors.  Note, however, that limited testing has
been done under Linux.  In particular, I am not certain about the accuracy of the timing
that is available.

