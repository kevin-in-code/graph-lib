How to build with Visual Studio 2015
------------------------------------

1. Install premake5.

2. On the command line, run:

    premake5 vs2015

3. Open the vs2015 project and build as Win64 Release.


How to build with GCC
---------------------

A Makefile is not currently included, but is easy to construct.
Simply compile each .cpp file in the src directory, as well as
those in the tests/* directories.  Link accordingly.

I haven't tested extensively under Linux or BSD.
