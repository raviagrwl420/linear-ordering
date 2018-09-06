#include<heuristic_callback.h>

void HeuristicCallback::postHeuristicSolution (const IloCplex::Callback::Context& context) {
	// Initialize a new variable to store the relaxation solution
	IloArray<IloNumArray> vars(context.getEnv(), size);
	for (int i = 0; i < size; i++) {
		vars[i] = IloNumArray(context.getEnv(), size);
	}

	// Initialize a new variable to store the new feasible solution
	IloArray<IloBoolArray> newSolution(context.getEnv(), size);
	for (int i = 0; i < size; i++) {
		newSolution[i] = IloBoolArray(context.getEnv(), size);
	}

	try {
		// Get the relaxation solution
		for (int i = 0; i < size; i++) {
			context.getRelaxationPoint(x[i], vars[i]);
		}

		// Get the relaxation objective
		double relaxationObjective = context.getRelaxationObjective();

		// Initialize new array for partial solution
		int** partialSolution = new int*[size];
		for (int i = 0; i < size; i++) {
			partialSolution[i] = new int[size];
		}

		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				partialSolution[i][j] = 0;
			}
		}

		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (vars[i][j] == 1) {
					partialSolution[i][j] = 1;
				}
			}
		}

		Ranking ranking(size, matrix);
		LinearOrder order = solvePartition(ranking, partialSolution);
		vector<int> orderVec = order.getOrder();

		double newObjective = ranking.getWeight(orderVec);

		for (int i = 0; i < size; i++) {
			for (int j = i + 1; j < size; j++) {
				newSolution[orderVec[i]][orderVec[j]] = 1;
			}
		}

		context.postHeuristicSolution(x, newSolution, newObjective, IloCplex::Callback::Context::SolutionStrategy::CheckFeasible);

		vars.end();
		newSolution.end();
	}
	catch (...) {
		vars.end();
		newSolution.end();
		throw;
	}
}

void HeuristicCallback::invoke (const IloCplex::Callback::Context& context) {
	if (context.inRelaxation()) {
		// Post Heuristic Solution
		postHeuristicSolution(context);
	}
}