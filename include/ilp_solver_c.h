#ifndef ILP_SOLVER_C
#define ILP_SOLVER_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ilcplex/cplexx.h>

extern "C" void dummy();
extern "C" int solve_c(int size, int** matrix);

#endif