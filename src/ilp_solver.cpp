#include<ilp_solver.h>

LinearOrder solveLP (Ranking& ranking) {
	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();

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

		cout << "Objective Value: " << cplex.getObjValue() << endl;
	} catch (IloException& e) {
		cout << "Exception: " << e << endl;
	}

	return NULL;
}

LinearOrder solveILP (Ranking& ranking) {
	// IloCplex::setParam(IloCplex::Param::MIP::Tolerances::Integrality, 0);

	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();

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

		cout << "Objective Value: " << cplex.getObjValue() << endl;

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
			cout << "Totals: " << totals[i] << endl;
			int index = (size - 1) - totals[i];
			orderArray[index] = i;
		}

		vector<int> orderVec;
		for (int i = 0; i < size; i++) {
			orderVec.push_back(orderArray[i]);
		}

		cout << "ILP Valid: " << LinearOrder(orderVec).validate() << endl;

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

LinearOrder solveRecursive(int size, int** partitionMatrix) {
	Ranking ranking(size, partitionMatrix);

	cout << endl << "**********Size**********" << endl << size << endl << endl;

	LinearOrder solved;
	if (size < 35) {
		solved = solveILP(ranking);
	} else {
		solved = solvePartition(ranking);
	}

	return solved;
}

LinearOrder solvePartition (Ranking& ranking) {
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

		cout << "Objective Value: " << cplex.getObjValue() << endl;

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

		int** partition1Matrix = extractSubMatrix(matrix, size1, partition1);
		int** partition2Matrix = extractSubMatrix(matrix, size2, partition2);

		LinearOrder order1 = solveRecursive(size1, partition1Matrix);
		LinearOrder order2 = solveRecursive(size2, partition2Matrix);

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

		cout << "Original Weight: " << ranking.getWeight(bestOrder.getOrder()) << endl;

		LinearOrder improvedOrder = localSearchExpensive(size, matrix, bestOrder);

		cout << "Improved Weight: " << ranking.getWeight(improvedOrder.getOrder()) << endl; 

		return improvedOrder;

		return LinearOrder(finalOrder);
	} catch (IloException& e) {
		cout << "Exception: " << e << endl;
	}

	return NULL;
}