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
#define LOCAL_ENUMERATION_WINDOW 8
#define RELAXATION_RATIO 1.0/3.0
#define MAX_CUTS_PER_ITERATION 500

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
				convex[i][j] = RELAXATION_RATIO * vars[i][j] + (1 - RELAXATION_RATIO) * bestInteger[i][j];
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

						if (count >= MAX_CUTS_PER_ITERATION) {
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

						if (count >= MAX_CUTS_PER_ITERATION) {
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

LinearOrder getSimpleHeuristicSolution(Ranking& ranking, float** fractionals) {
	int size = ranking.getSize();

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

	// Reverse
	vector<int> reverseOrderVec;
	for (int i = 1; i <= size; i++) {
		reverseOrderVec.push_back(order[size - i]);
	}

	LinearOrder initialOrder(reverseOrderVec);

	return localEnumeration(ranking, initialOrder, LOCAL_ENUMERATION_WINDOW);
}

LinearOrder getOurHeuristicSolution(Ranking& ranking, float** fractionals) {
	int size = ranking.getSize();
	return solvePartition(size, fractionals, ranking);
}

LinearOrder getOurAlternateHeuristicSolution(Ranking& ranking, int** partialSolution) {
	int size = ranking.getSize();
	return solvePartition(ranking, partialSolution);
}

bool isFeasible (Ranking& ranking, IloArray<IloNumArray> vars) {
	int size = ranking.getSize();

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

	freeMemory2D(partialSolution, size);

	cout << "Num Components: " << numComponents << " Size: " << size << endl;

	return numComponents == size;
}

LinearOrder getHeuristicSolution (Ranking& ranking, IloArray<IloNumArray> vars) {
	int size = ranking.getSize();

	float** fractionals = new float*[size];
	for (int i = 0; i < size; i++) {
		fractionals[i] = new float[size];
		for (int j = 0; j < size; j++) {
			fractionals[i][j] = vars[i][j];
		}
	}

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

	// Compute Heuristic Solutions
	LinearOrder simple = getSimpleHeuristicSolution(ranking, fractionals);
	LinearOrder ours = getOurHeuristicSolution(ranking, fractionals);
	ours = localSearchExpensive(ranking, ours);
	LinearOrder oursAlternate = getOurAlternateHeuristicSolution(ranking, partialSolution);
	oursAlternate = localSearchExpensive(ranking, oursAlternate);

	cout << endl << endl;
	cout << "Simple Objective: " << ranking.getWeight(simple) << endl;
	cout << "Ours Objective: " << ranking.getWeight(ours) << endl;
	cout << "Ours Alternate Objective: " << ranking.getWeight(oursAlternate) << endl;
	cout << endl << endl;

	freeMemory2D(fractionals, size);
	freeMemory2D(partialSolution, size);

	return ours;
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

		// Compute Heuristic Solution
		if (isFeasible(ranking, vars)) {
			LinearOrder order = getHeuristicSolution(ranking, vars);

			// Decode Heuristic Solution
			for (int i = 0; i < size; i++) {
				for (int j = i + 1; j < size; j++) {
					newSolution[order[i] * size + order[j]] = 1;
				}
			}
			int newObjective = ranking.getWeight(order);

			// Set Solution
			setSolution(flattenedX, newSolution, newObjective);
		}

		// Free Memory
		freeMemory(vars);
		flattenedX.end();
		newSolution.end();
	} catch (IloException& e) {
		cout << "LegacyNodeCallbackException: " << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}

	return;
}

#endif