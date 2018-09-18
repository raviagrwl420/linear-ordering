#ifndef LEGACY_CALLBACK_H
#define LEGACY_CALLBACK_H

#include<ilp_solver.h>
#include<ranking.h>
#include<io.h>
#include<graph.h>

#include<ilcplex/ilocplex.h>
#include<vector>

#define EPS 1e-4

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
		for (int i = 0; i < size; i++) {
			vars[i].end();
		}
		vars.end();
	} catch (IloException& e) {
		cout << "LegacyLazyConstraintCallbackException: " << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}

	return;
}

ILOUSERCUTCALLBACK4(LegacyUserCutCallback, IloCplex, cplex, IloArray<IloBoolVarArray>, x, IloObjective, obj, Ranking, ranking) {
	float relaxationRatio = 1.0 / 3;
	// int maxCutCount = 100;

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
					for (int j = 0; j < size; j++) {
						bestInteger[i][j] = 0;
					}	
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
		for (int i = 0; i < size; i++) {
			for (int j = i + 1; j < size; j++) {
				for (int k = i + 1; k < size; k++) {
					if (j != k) {
						if (cutOffPoint[i][j] + cutOffPoint[j][k] + cutOffPoint[k][i] > 2 + EPS) {
							add(x[i][j] + x[j][k] + x[k][i] <= 2).end();
							count++;

							// if (count > maxCutCount) {
							// 	goto end;
							// }
						}
					}
				}
			}
		}

		// If convex combination cannot be cut off then cut off relaxation
		if (count == 0) {
			cutOffPoint = vars;
			for (int i = 0; i < size; i++) {
				for (int j = i + 1; j < size; j++) {
					for (int k = i + 1; k < size; k++) {
						if (j != k) {
							if (cutOffPoint[i][j] + cutOffPoint[j][k] + cutOffPoint[k][i] > 2 + EPS) {
								add(x[i][j] + x[j][k] + x[k][i] <= 2).end();
								count++;

								// if (count > maxCutCount) {
								// 	goto end;
								// }
							}
						}
					}
				}
			}
		}

		end:;

		// Free Memory
		for (int i = 0; i < size; i++) {
			vars[i].end();
			bestInteger[i].end();
			convex[i].end();
		}
		vars.end();
		bestInteger.end();
		convex.end();
	} catch (IloException& e) {
		cout << "LegacyUserCutCallbackException: " << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}

	return;
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
		IloIntVarArray flattenedX(env, size * size);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				flattenedX[size * i + j] = x[i][j];
			}
		}

		// Initialize a new variable to store the new feasible solution
		IloNumArray newSolution(env, size * size);

		// Initialize new array for partial solution
		// int** partialSolution = new int*[size];
		// for (int i = 0; i < size; i++) {
		// 	partialSolution[i] = new int[size];
		// }

		// for (int i = 0; i < size; i++) {
		// 	for (int j = 0; j < size; j++) {
		// 		partialSolution[i][j] = 0;
		// 	}
		// }

		// for (int i = 0; i < size; i++) {
		// 	for (int j = 0; j < size; j++) {
		// 		if ( 1 - EPS <= vars[i][j]) {
		// 			partialSolution[i][j] = 1;
		// 		}
		// 	}
		// }

		// Temp
		// Ranking partialRanking(size, partialSolution);
		// int numComponents = strongly_connected_components(partialRanking);
		// if (numComponents < size) {
		// 	cout << "Size: "<< size << " Components: " << numComponents << endl;
		// 	return;
		// }

		float** fractionals = new float*[size];
		for (int i = 0; i < size; i++) {
			fractionals[i] = new float[size];
			for (int j = 0; j < size; j++) {
				fractionals[i][j] = vars[i][j];
			}
		}

		// Compute Heuristic Solution
		// Ranking ranking(size, matrix);
		LinearOrder order = solvePartition(size, fractionals, ranking);
		vector<int> orderVec = order.getOrder();

		double newObjective = ranking.getWeight(orderVec);
		cout << endl << endl << "Feasible Objective: " << newObjective << endl << endl << endl;

		// Decode Heuristic Solution
		for (int i = 0; i < size; i++) {
			for (int j = i + 1; j < size; j++) {
				newSolution[orderVec[i] * size + orderVec[j]] = 1;
			}
		}

		// Set Solution
		setSolution(flattenedX, newSolution, newObjective);

		// Free Memory
		for (int i = 0; i < size; i++) {
			vars[i].end();
			delete [] fractionals[i];
		}
		vars.end();
		flattenedX.end();
		newSolution.end();
		delete [] fractionals;
	} catch (IloException& e) {
		cout << "LegacyNodeCallbackException: " << e.getMessage() << endl;
	} catch (...) {
		cout << "GenericException!" << endl;
	}

	return;
}

#endif