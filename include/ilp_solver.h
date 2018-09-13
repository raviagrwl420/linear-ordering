#ifndef ILP_SOLVER_H
#define ILP_SOLVER_H

#include<linear_order.h>
#include<ranking.h>
#include<local_search.h>
#include<generic_callback.h>
#include<heuristic_callback.h>

#include<ilcplex/ilocplex.h>
#include<math.h>

float** solveLP (Ranking&);
LinearOrder solveILP (Ranking&, bool disableOutput);
LinearOrder solvePartition (Ranking&, int** partialSolution);
LinearOrder solveTriPartition (Ranking& ranking, int** partialSolution);
LinearOrder solvePartition (int size, float** fractionals, Ranking& ranking);

#endif