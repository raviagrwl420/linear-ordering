#include<ilp_solver.h>

float** solveLP (Ranking& ranking) {
	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();

	// Initialize a new 2D array to store the fractional values of the decision variables
	float** fractionals = new float*[size];
	for (int i = 0; i < size; i++) {
		fractionals[i] = new float[size];
	}

	try {
		// Initialize environment and model
		IloEnv env;
		IloModel model(env);
		IloCplex cplex(model);

		// Initialize variable array
		IloArray<IloNumVarArray> x(env, size);
		for (int i = 0; i < size; i++) {
			x[i] = IloNumVarArray(env, size, 0, 1, ILOFLOAT);
		}

		// Initialize objective function
		IloExpr obj(env);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					obj += (matrix[i][j] * x[i][j]);
				}
			}
		}
		model.add(IloMaximize(env, obj));

		// Add simple constraints
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					model.add(x[i][j] + x[j][i] == 1);
				} else {
					model.add(x[i][i] == 0);
				}
			}
		}

		// Add cycle constraints
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				for (int k = 0; k < size; k++) {
					if ((i != j) && (j != k) && (k != i)) {
						model.add(x[i][j] + x[j][k] + x[k][i] <= 2);
					}
				}
			}
		}

		// Solve
		cplex.solve();

		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				fractionals[i][j] = cplex.getValue(x[i][j]);
			}
		}

		// cout << "Objective Value: " << cplex.getObjValue() << endl;

		return fractionals;
	} catch (IloException& e) {
		cout << "Exception: " << e << endl;
	}

	return NULL;
}

LinearOrder solveILP (Ranking& ranking, bool disableOutput) {
	// IloCplex::setParam(IloCplex::Param::MIP::Tolerances::Integrality, 0);
	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();

	try {
		// Initialize environment and model
		IloEnv env;
		IloModel model(env);
		IloCplex cplex(model);

		if (disableOutput) {
			cplex.setOut(env.getNullStream());
		}

		cplex.setParam(IloCplex::Param::MIP::Tolerances::Integrality, 0);
		cplex.setParam(IloCplex::Param::MIP::Strategy::Search, 1);
		// cplex.setParam(IloCplex::Param::MIP::Strategy::HeuristicFreq, 5);

		// Initialize variable array
		IloArray<IloBoolVarArray> x(env, size);
		for (int i = 0; i < size; i++) {
			x[i] = IloBoolVarArray(env, size);
		}

		// Flatten x
		IloNumVarArray flattenedX(env, size * size);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				flattenedX[i * size + j] = x[i][j];
			}
		}

		// Add callback
		HeuristicCallback cb(x, matrix);
		cplex.use(&cb, IloCplex::Callback::Context::Id::Relaxation);

		// Initialize objective function
		IloExpr obj(env);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					obj += (matrix[i][j] * x[i][j]);
				}
			}
		}
		model.add(IloMaximize(env, obj));

		// Add simple constraints
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					model.add(x[i][j] + x[j][i] == 1);
				} else {
					model.add(x[i][i] == 0);
				}
			}
		}

		// Add cycle constraints
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				for (int k = 0; k < size; k++) {
					if ((i != j) && (j != k) && (k != i)) {
						model.add(x[i][j] + x[j][k] + x[k][i] <= 2);
					}
				}
			}
		}

		// Add MIP Start
		if (size > 35) {
			IloNumArray startSolution(env, size * size);
			LinearOrder startOrder = solvePartition(ranking, NULL);
			vector<int> startOrderVec = startOrder.getOrder();

			cout << "Start Weight: " << ranking.getWeight(startOrderVec);

			for (int i = 0; i < size; i++) {
				for (int j = i + 1; j < size; j++) {
					startSolution[startOrderVec[i] * size + startOrderVec[j]] = 1;
				}
			}

			cplex.addMIPStart(flattenedX, startSolution);
			flattenedX.end();
			startSolution.end();	
		}

		// Solve
		cplex.solve();

		// cout << "Objective Value: " << cplex.getObjValue() << endl;

		// Decode the solution
		// Add the values for each row
		int* totals = new int[size];
		for (int i = 0; i < size; i++) {
			int total = 0;
			for (int j = 0; j < size; j++) {
				total += cplex.getValue(x[i][j]);
			}
			totals[i] = total;
		}

		// Insert the node in its proper position in the order
		int* orderArray = new int[size];
		for (int i = 0; i < size; i++) {
			// cout << "Totals: " << totals[i] << endl;
			int index = (size - 1) - totals[i];
			orderArray[index] = i;
		}

		vector<int> orderVec;
		for (int i = 0; i < size; i++) {
			orderVec.push_back(orderArray[i]);
		}

		// cout << "ILP Valid: " << LinearOrder(orderVec).validate() << endl;

		return LinearOrder(orderVec);
	} catch (IloException& e) {
		cout << "Exception: " << e << endl;
	}

	return NULL;
}

int** extractSubMatrix (int** matrix, int size, int* nodes) {
	int** newMatrix = new int*[size];
	for (int i = 0; i < size; i++) {
		newMatrix[i] = new int[size];
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			newMatrix[i][j] = matrix[nodes[i]][nodes[j]];
		}
	}

	return newMatrix;
}

LinearOrder solveRecursive(int size, int** partitionMatrix, int** partialSolution) {
	Ranking ranking(size, partitionMatrix);

	// cout << endl << "**********Size**********" << endl << size << endl << endl;

	LinearOrder solved;
	if (size < 35) {
		solved = solveILP(ranking, true);
	} else {
		solved = solvePartition(ranking, partialSolution);
	}

	return solved;
}

LinearOrder solvePartition (Ranking& ranking, int** partialSolution) {
	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();

	try {
		// Initialize environment and model
		IloEnv env;
		IloModel model(env);
		IloCplex cplex(model);

		// Initialize variable array
		IloBoolVarArray x(env, size);

		// Add the variable to model to force its extraction
		model.add(x);

		// Initialize variable array
		IloArray<IloBoolVarArray> y(env, size);
		for (int i = 0; i < size; i++) {
			y[i] = IloBoolVarArray(env, size);
		}

		// Initialize objective function
		IloExpr obj(env);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					// obj += (matrix[i][j] * (x[i] - x[j]));
					obj += (matrix[i][j] * y[i][j]);
				}
			}
		}
		model.add(IloMaximize(env, obj));

		// Add constraints
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					model.add(y[i][j] <= x[i]);
					model.add(y[i][j] <= 1 - x[j]);	
				}
			}
		}

		// Add partial solution constraints
		if (partialSolution != NULL) {
			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) {
					if (partialSolution[i][j] == 1) {
						model.add(x[i] - x[j] >= 0);
					}
				}
			}
		}

		// Solve
		cplex.solve();

		// cout << "Objective Value: " << cplex.getObjValue() << endl;

		// Decode solution
		// Count number of nodes in partition
		int count = 0;
		int* values = new int[size];
		for (int i = 0; i < size; i++) {
			values[i] = cplex.getValue(x[i]);
			count +=  values[i];
		}

		int size1 = count;
		int size2 = size - count;
		int* partition1 = new int[size1];
		int* partition2 = new int[size2];

		int j = 0;
		int k = 0;
		for (int i = 0; i < size; i++) {
			if (values[i] == 1) {
				partition1[j] = i;
				j++;
			} else {
				partition2[k] = i;
				k++;
			}
		}

		cout << "Count: " << count << " Size: " << size << endl;

		int** partition1Matrix = extractSubMatrix(matrix, size1, partition1);
		int** partialSolution1Matrix = partialSolution != NULL ? extractSubMatrix(partialSolution, size1, partition1) : NULL;
		int** partition2Matrix = extractSubMatrix(matrix, size2, partition2);
		int** partialSolution2Matrix = partialSolution != NULL ? extractSubMatrix(partialSolution, size2, partition2) : NULL;

		LinearOrder order1 = solveRecursive(size1, partition1Matrix, partialSolution1Matrix);
		LinearOrder order2 = solveRecursive(size2, partition2Matrix, partialSolution2Matrix);

		vector<int> order1Vec = order1.getOrder();
		vector<int> order2Vec = order2.getOrder();

		vector<int> finalOrder;
		for (int i = 0; i < size1; i++) {
			finalOrder.push_back(partition1[order1Vec[i]]);
		}
		for (int i = 0; i < size2; i++) {
			finalOrder.push_back(partition2[order2Vec[i]]);
		}

		// LinearOrder bestOrder(finalOrder);

		// cout << "Original Weight: " << ranking.getWeight(bestOrder.getOrder()) << endl;

		// LinearOrder improvedOrder = localSearchExpensive(size, matrix, bestOrder);

		// cout << "Improved Weight: " << ranking.getWeight(improvedOrder.getOrder()) << endl; 

		// return improvedOrder;

		return LinearOrder(finalOrder);
	} catch (IloException& e) {
		cout << "Exception: " << e << endl;
	}

	return NULL;
}

LinearOrder solveILP (int size, float** matrix) {
	try {
		// Initialize environment and model
		IloEnv env;
		IloModel model(env);
		IloCplex cplex(model);

		cplex.setParam(IloCplex::Param::MIP::Tolerances::Integrality, 0);

		// Initialize variable array
		IloArray<IloBoolVarArray> x(env, size);
		for (int i = 0; i < size; i++) {
			x[i] = IloBoolVarArray(env, size);
		}

		// Initialize objective function
		IloExpr obj(env);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					obj += (matrix[i][j] * x[i][j]);
				}
			}
		}
		model.add(IloMaximize(env, obj));

		// Add simple constraints
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					model.add(x[i][j] + x[j][i] == 1);
				} else {
					model.add(x[i][i] == 0);
				}
			}
		}

		// Add cycle constraints
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				for (int k = 0; k < size; k++) {
					if ((i != j) && (j != k) && (k != i)) {
						model.add(x[i][j] + x[j][k] + x[k][i] <= 2);
					}
				}
			}
		}

		// Solve
		cplex.solve();

		// cout << "Objective Value: " << cplex.getObjValue() << endl;

		// Decode the solution
		// Add the values for each row
		int* totals = new int[size];
		for (int i = 0; i < size; i++) {
			int total = 0;
			for (int j = 0; j < size; j++) {
				total += cplex.getValue(x[i][j]);
			}
			totals[i] = total;
		}

		// Insert the node in its proper position in the order
		int* orderArray = new int[size];
		for (int i = 0; i < size; i++) {
			// cout << "Totals: " << totals[i] << endl;
			int index = (size - 1) - totals[i];
			orderArray[index] = i;
		}

		vector<int> orderVec;
		for (int i = 0; i < size; i++) {
			orderVec.push_back(orderArray[i]);
		}

		// cout << "ILP Valid: " << LinearOrder(orderVec).validate() << endl;

		return LinearOrder(orderVec);
	} catch (IloException& e) {
		cout << "Exception: " << e << endl;
	}

	return NULL;
}

float** extractSubMatrix (float** matrix, int size, int* nodes) {
	float** newMatrix = new float*[size];
	for (int i = 0; i < size; i++) {
		newMatrix[i] = new float[size];
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			newMatrix[i][j] = matrix[nodes[i]][nodes[j]];
		}
	}

	return newMatrix;
}

LinearOrder solveRecursive(int size, float** partitionMatrix, int** intMatrix) {
	Ranking newRanking(size, intMatrix);

	LinearOrder solved;
	if (size < 40) {
		solved = solveILP(newRanking, true);
	} else {
		solved = solvePartition(size, partitionMatrix, newRanking);
	}

	return solved;
}

LinearOrder solvePartition (int size, float** matrix, Ranking& ranking) {
	try {
		// Initialize environment and model
		IloEnv env;
		IloModel model(env);
		IloCplex cplex(model);

		// Initialize variable array
		IloBoolVarArray x(env, size);

		// Add the variable to model to force its extraction
		model.add(x);

		// Initialize objective function
		IloExpr obj(env);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					obj += (matrix[i][j] * (x[i] - x[j]));
				}
			}
		}
		model.add(IloMaximize(env, obj));

		// Solve
		cplex.solve();

		// cout << "Objective Value: " << cplex.getObjValue() << endl;

		// Decode solution
		// Count number of nodes in partition
		int count = 0;
		int* values = new int[size];
		for (int i = 0; i < size; i++) {
			values[i] = cplex.getValue(x[i]);
			count +=  values[i];
		}

		int size1 = count;
		int size2 = size - count;
		int* partition1 = new int[size1];
		int* partition2 = new int[size2];

		int j = 0;
		int k = 0;
		for (int i = 0; i < size; i++) {
			if (values[i] == 1) {
				partition1[j] = i;
				j++;
			} else {
				partition2[k] = i;
				k++;
			}
		}

		float** partition1Matrix = extractSubMatrix(matrix, size1, partition1);
		float** partition2Matrix = extractSubMatrix(matrix, size2, partition2);

		int** intMatrix = ranking.getMatrix();
		int** intPartitionMatrix1 = extractSubMatrix(intMatrix, size1, partition1);
		int** intPartitionMatrix2 = extractSubMatrix(intMatrix, size2, partition2);

		LinearOrder order1 = solveRecursive(size1, partition1Matrix, intPartitionMatrix1);
		LinearOrder order2 = solveRecursive(size2, partition2Matrix, intPartitionMatrix2);

		vector<int> order1Vec = order1.getOrder();
		vector<int> order2Vec = order2.getOrder();

		vector<int> finalOrder;
		for (int i = 0; i < size1; i++) {
			finalOrder.push_back(partition1[order1Vec[i]]);
		}
		for (int i = 0; i < size2; i++) {
			finalOrder.push_back(partition2[order2Vec[i]]);
		}

		LinearOrder bestOrder(finalOrder);

		// cout << "Original Weight: " << ranking.getWeight(bestOrder.getOrder()) << endl;

		LinearOrder improvedOrder = localSearchExpensive(size, intMatrix, bestOrder);

		// cout << "Improved Weight: " << ranking.getWeight(improvedOrder.getOrder()) << endl; 

		return improvedOrder;

		return LinearOrder(finalOrder);
	} catch (IloException& e) {
		cout << "Exception: " << e << endl;
	}

	return NULL;
}