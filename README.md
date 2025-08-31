# Build-System
A simple build system to make building projects across multiple machines easier for projects with long command line arguments.
In order to set it up, simply clone the repository, and compile the single c file on your machine using the compiler of your choice. The program only uses functions from the standard c libraries, specifically string.h, stdio.h, stdlib.h, and stdbool.h. This means it should work on most operating systems.
An example for how the compilation would look is: gcc buildsys.c -o build
