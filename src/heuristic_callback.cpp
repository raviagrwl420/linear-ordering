#include<heuristic_callback.h>

#define EPS 1e-4

void HeuristicCallback::postHeuristicSolution (const IloCplex::Callback::Context& context) {
	// Initialize a new variable to store the relaxation solution
	IloArray<IloNumArray> vars(context.getEnv(), size);
	for (int i = 0; i < size; i++) {
		vars[i] = IloNumArray(context.getEnv(), size);
	}

	// Flatten x array
	IloIntVarArray flattenedX(context.getEnv(), size * size);
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			flattenedX[size * i + j] = x[i][j];
		}
	}

	// Initialize a new variable to store the new feasible solution
	IloNumArray newSolution(context.getEnv(), size * size);
	// for (int i = 0; i < size; i++) {
	// 	newSolution[i] = IloBoolArray(context.getEnv(), size);
	// }

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
				if ( 1 - EPS <= vars[i][j] && vars[i][j] <= 1 + EPS) {
					partialSolution[i][j] = 1;
				}
			}
		}

		Ranking ranking(size, matrix);
		LinearOrder order = solvePartition(ranking, partialSolution);
		vector<int> orderVec = order.getOrder();

		double newObjective = ranking.getWeight(orderVec);

		cout << endl << endl << "Feasible Objective: " << newObjective << endl << endl << endl;

		for (int i = 0; i < size; i++) {
			for (int j = i + 1; j < size; j++) {
				newSolution[orderVec[i] * size + orderVec[j]] = 1;
			}
		}

		context.postHeuristicSolution(flattenedX, newSolution, newObjective, IloCplex::Callback::Context::SolutionStrategy::CheckFeasible);

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
	// if (context.inRelaxation()) {
		// Post Heuristic Solution
		postHeuristicSolution(context);
	// }
}