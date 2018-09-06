#ifndef ILP_SOLVER_H
#define ILP_SOLVER_H

#include<linear_order.h>
#include<ranking.h>
#include<local_search.h>
#include<generic_callback.h>

#include<ilcplex/ilocplex.h>

float** solveLP (Ranking&);
LinearOrder solveILP (Ranking&);
LinearOrder solvePartition (Ranking&);
LinearOrder solvePartition (int size, float** fractionals, Ranking& ranking);

#endif