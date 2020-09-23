#####################################################################
# CS:APP Malloc Lab
# Handout files for students
#
# Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
# May not be used, modified, or copied without permission.
#
######################################################################

***********
Main Files:
***********

mdriver.c	
	The malloc driver that tests your mm.c file

short{1,2}-bal.rep
	Two tiny tracefiles to help you get started. 

Makefile	
	Builds the driver

**********************************
Other support files for the driver
**********************************

config.h	Configures the malloc lab driver
fsecs.{c,h}	Wrapper function for the different timer packages
clock.{c,h}	Routines for accessing the Pentium and Alpha cycle counters
fcyc.{c,h}	Timer functions based on cycle counters
ftimer.{c,h}	Timer functions based on interval timers and gettimeofday()
memlib.{c,h}	Models the heap and sbrk function

*******************************
Building and running the driver
*******************************
To build the driver, type "make" to the shell.

To run the driver on a tiny test trace: (for the naive implementation)

	shell> ./mdriver -V -f traces/short1-bal.rep
	
To run the driver on a tiny test trace: (for the mm1.c)

	shell> ./mdriver1 -V -f traces/short1-bal.rep
	
To run the driver on a tiny test trace: (for the mm2.c)

	shell> ./mdriver2 -V -f traces/short1-bal.rep

The -V option prints out helpful tracing and summary information.

To get a list of the driver flags:

	shell> ./mdriver -h

