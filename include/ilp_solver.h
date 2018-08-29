#ifndef ILP_SOLVER_H
#define ILP_SOLVER_H

#include<linear_order.h>
#include<ranking.h>

#include<ilcplex/ilocplex.h>

LinearOrder solveLP (Ranking&);
LinearOrder solveILP (Ranking&);
LinearOrder solvePartition (Ranking&);

#endif