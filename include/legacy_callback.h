#ifndef LEGACY_CALLBACK_H
#define LEGACY_CALLBACK_H

#include<ilp_solver.h>
#include<ranking.h>
#include<io.h>
#include<graph.h>
#include<constraint.h>
#include<local_search.h>
#include<memory_management.h>

#include<ilcplex/ilocplex.h>
#include<vector>

#define EPS 1e-8

void freeMemory (IloArray<IloNumArray> vars) {
	int size = vars.getSize();
	for (int i = 0; i < size; i++) {
		vars[i].end();
	}
	vars.end();
}

ILOLAZYCONSTRAINTCALLBACK4(LegacyLazyConstraintCallback, IloCplex, cplex, IloArray<IloBoolVarArray>, x, IloObjective, obj, Ranking, ranking) {
	try {
		// Get size and matrix from ranking
		int size = ranking.getSize();
		int** matrix = ranking.getMatrix();

		// Get env using internal method
		IloEnv env = getEnv();

		// Initialize a new variable to store the relaxation solution
		IloArray<IloNumArray> vars(env, size);
		for (int i = 0; i < size; i++) {
			vars[i] = IloNumArray(env, size);
			// Get values using internal method
			getValues(vars[i], x[i]);
		}

		for (int i = 0; i < size; i++) {
			for (int j = i + 1; j < size; j++) {
				for (int k = i + 1; k < size; k++) {
					if (j != k) {
						if (vars[i][j] + vars[j][k] + vars[k][i] > 2 + EPS) {
							add(x[i][j] + x[j][k] + x[k][i] <= 2).end();	
						}
					}
				}
			}
		}

		// Free Memory
		freeMemory(vars);
	} catch (IloException& e) {
		cout << "LegacyLazyConstraintCallbackException: " << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}

	return;
}

ILOUSERCUTCALLBACK5(LegacyUserCutCallback, IloCplex, cplex, IloArray<IloBoolVarArray>, x, IloObjective, obj, Ranking, ranking, vector<Constraint>, constraints) {
	if (!isAfterCutLoop()) {
		return;
	}

	try {
		float relaxationRatio = 1.0 / 3;
		int maxCutCount = 500;

		// Get size and matrix from ranking
		int size = ranking.getSize();
		int** matrix = ranking.getMatrix();

		// Get env using internal method
		IloEnv env = getEnv();

		// Initialize a new variables to store the relaxation and best incumbent
		bool integerSolutionExists = true;
		IloArray<IloNumArray> vars(env, size);
		IloArray<IloNumArray> bestInteger(env, size);
		for (int i = 0; i < size; i++) {
			vars[i] = IloNumArray(env, size);
			bestInteger[i] = IloNumArray(env, size);
			
			// Get values using internal methods
			getValues(vars[i], x[i]);

			try {
				getIncumbentValues(bestInteger[i], x[i]);	
			} catch (IloCplex::Exception& e) {
				// If no solution exists.
				if (e.getStatus() == 1217) {
					integerSolutionExists = false;
				}
				e.end();
			}
		}

		// Compute the convex combination of relaxation and best incumbent
		IloArray<IloNumArray> convex(env, size);
		for (int i = 0; i < size; i++) {
			convex[i] = IloNumArray(env, size);
			for (int j = 0; j < size; j++) {
				convex[i][j] = relaxationRatio * vars[i][j] + (1 - relaxationRatio) * bestInteger[i][j];
			}
		}

		// Try to cut off convex combination of relaxation and best incumbent
		IloArray<IloNumArray> cutOffPoint = convex;
		int count = 0;

		if (integerSolutionExists) {
			for (Constraint& constraint: constraints) {
				if (constraint.active) {
					int i = constraint.i;
					int j = constraint.j;
					int k = constraint.k;

					if (cutOffPoint[i][j] + cutOffPoint[j][k] + cutOffPoint[k][i] > 2 + EPS) {
						add(x[i][j] + x[j][k] + x[k][i] <= 2, IloCplex::CutManagement::UseCutPurge).end();
						constraint.deactivate();
						count++;

						if (count > maxCutCount) {
							goto end;
						}
					}
				}
			}
		}

		// If convex combination cannot be cut off then cut off relaxation
		if (count == 0) {
			cutOffPoint = vars;
			for (Constraint& constraint: constraints) {
				if (constraint.active) {
					int i = constraint.i;
					int j = constraint.j;
					int k = constraint.k;

					if (cutOffPoint[i][j] + cutOffPoint[j][k] + cutOffPoint[k][i] > 2 + EPS) {
						add(x[i][j] + x[j][k] + x[k][i] <= 2, IloCplex::CutManagement::UseCutPurge).end();
						constraint.deactivate();
						count++;

						if (count > maxCutCount) {
							goto end;
						}
					}
				}
			}
		}

		end:;

		// Free Memory
		freeMemory(vars);
		freeMemory(bestInteger);
		freeMemory(convex);
	} catch (IloException& e) {
		cout << "LegacyUserCutCallbackException: " << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}

	return;
}

LinearOrder getSimpleHeuristicSolution(Ranking& ranking, int size, float** fractionals) {
	vector<float> totals(size);

	for (int i = 0; i < size; i++) {
		totals[i] = 0;
		for (int j = 0; j < size; j++) {
			if (i != j) {
				totals[i] += fractionals[i][j];	
			}
		}
	}

	LinearOrder order(totals);
	vector<int> orderVec = order.getOrder();

	// Reverse
	vector<int> reverseOrderVec;
	for (int i = 1; i <= size; i++) {
		reverseOrderVec.push_back(orderVec[size - i]);
	}

	return LinearOrder(reverseOrderVec);
}

LinearOrder getOurHeuristicSolution(Ranking& ranking, int size, float** fractionals) {
	return solvePartition(size, fractionals, ranking);
}

LinearOrder getOurAlternateHeuristicSolution(Ranking& ranking, int size, IloArray<IloNumArray> vars) {
	// Initialize new array for partial solution
	int** partialSolution = new int*[size];
	for (int i = 0; i < size; i++) {
		partialSolution[i] = new int[size];
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if ( 1 - EPS <= vars[i][j]) {
				partialSolution[i][j] = 1;
			} else {
				partialSolution[i][j] = 0;
			}
		}
	}

	// Temp
	Ranking partialRanking(size, partialSolution);
	int numComponents = strongly_connected_components(partialRanking);
	if (numComponents < size) {
		return LinearOrder(0);
	}

	LinearOrder order = solvePartition(ranking, partialSolution);

	freeMemory2D(partialSolution, size);

	return order;
}

ILOHEURISTICCALLBACK4(LegacyHeuristicCallback, IloCplex, cplex, IloArray<IloBoolVarArray>, x, IloObjective, obj, Ranking, ranking) {
	try {
		// Get size and matrix from ranking
		int size = ranking.getSize();
		int** matrix = ranking.getMatrix();

		// Get env using internal method
		IloEnv env = getEnv();

		// Initialize a new variable to store the relaxation solution
		IloArray<IloNumArray> vars(env, size);
		for (int i = 0; i < size; i++) {
			vars[i] = IloNumArray(env, size);
			// Get values using internal method
			getValues(vars[i], x[i]);
		}

		// Flatten x array
		// TODO: Pass the flattened X array
		IloIntVarArray flattenedX(env, size * size);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				flattenedX[size * i + j] = x[i][j];
			}
		}

		// Initialize a new variable to store the new feasible solution
		IloNumArray newSolution(env, size * size);

		float** fractionals = new float*[size];
		for (int i = 0; i < size; i++) {
			fractionals[i] = new float[size];
			for (int j = 0; j < size; j++) {
				fractionals[i][j] = vars[i][j];
			}
		}

		// Compute Heuristic Solution
		LinearOrder simpleOrder = getSimpleHeuristicSolution(ranking, size, fractionals);
		LinearOrder ourOrder = getOurHeuristicSolution(ranking, size, fractionals);
		LinearOrder ourAlternateOrder = getOurAlternateHeuristicSolution(ranking, size, vars);
		vector<int> simpleOrderVec = simpleOrder.getOrder();
		vector<int> ourOrderVec = ourOrder.getOrder();

		double simpleObjective = ranking.getWeight(simpleOrderVec);
		double ourObjective = ranking.getWeight(ourOrderVec);
		cout << endl << endl << "Feasible Objective simpleObjective: " << simpleObjective << endl;
		cout << "Feasible Objective Ours: " << ourObjective << endl;

		if (ourAlternateOrder.getOrder().size() != 0) {
			vector<int> ourAlternateOrderVec = ourAlternateOrder.getOrder();
			double ourAlternateObjective = ranking.getWeight(ourAlternateOrderVec);
			cout << "Feasible Objective Ours Alternate: " << ourAlternateObjective << endl;
		}

		cout << endl << endl;

		vector<int> orderVec = ourOrderVec;
		double newObjective = ranking.getWeight(orderVec);

		// Decode Heuristic Solution
		for (int i = 0; i < size; i++) {
			for (int j = i + 1; j < size; j++) {
				newSolution[orderVec[i] * size + orderVec[j]] = 1;
			}
		}

		// Set Solution
		setSolution(flattenedX, newSolution, newObjective);

		// Free Memory
		freeMemory(vars);
		flattenedX.end();
		newSolution.end();
		freeMemory2D(fractionals, size);
	} catch (IloException& e) {
		cout << "LegacyNodeCallbackException: " << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}

	return;
}

#endif