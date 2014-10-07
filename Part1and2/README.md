# Introduction #

This is the code for Project 2 of COP4610 Fall 14 at Florida State University.

# Author #

Written by Ian Donnelly.

# Part 1 #

Part 1 consists of the file `tracetest.c` which produces 20 more system calls than a blank C program (46 instead of 26 as reported by `strace`).

# Part 2 #

Part 2 consists of the file `my_date.c` which is a kernel module that when mounted will create the file `/prog/currentdate` which will contain the current date and time. It can be built by running `make`.