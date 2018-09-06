#ifndef HEURISTIC_CALLBACK_H
#define HEURISTIC_CALLBACK_H

#include<ilp_solver.h>
#include<io.h>

#include<ilcplex/ilocplex.h>
#include<vector>

class HeuristicCallback: public IloCplex::Callback::Function {
	public:
		IloArray<IloBoolVarArray> x;
		int** matrix;
		int size;

		HeuristicCallback (IloArray<IloBoolVarArray> x, int** matrix): x(x), matrix(matrix), size(x.getSize()) {};

		void postHeuristicSolution (const IloCplex::Callback::Context& context);

		virtual void invoke (const IloCplex::Callback::Context& context);
};

#endif