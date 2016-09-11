GraphLib
--------

This repository provides code that may be run to verify published experimental
results.  It is not intended for inclusion into production projects, although
that is fairly easy to do.

The code is organised as follows:
    include     - contains the headers of the library
    src         - contains the main library source code
    programs/*  - each directory contains a small example, test, or experiment

Examples include:
    Main_TCS_2015       - experiments performed for my TCS 2015 publication
    CountMaximalCliques - a simple demonstration program
    Benchmarks          - a collection of clique enumeration benchmarks

The demonstration CountMaximalCliques program can apply clique enumeration to a
graph loaded from a file.  The graph file must be in one of several common text
formats.  The following simple graphs are provided for testing:

    programs/CountMaximalCliques/graph.am.csv,
    programs/CountMaximalCliques/graph.al.csv, and
    programs/CountMaximalCliques/graph.dimacs.txt.

In general, compiling the software should be straightforward.  Build scripts are
provided in the form of Makefiles for GNU Make, and premake5 lua scripts.

The software should compile without errors, although cross-platform development
may provide occassional surprises.  Building under Microsoft Visual Studio,
MinGW G++, and Linux G++ should nevertheless succeed.

How to build with Visual Studio 2015
------------------------------------

1. Install premake5.

2. On the command line, run:

    premake5 vs2015

3. Open the vs2015 project and build as Win64 Release.


How to build with GCC
---------------------

1. make

Makefile defaults to a release build.  Use "make debug" for a debug build.



