#include<ilp_solver.h>

LinearOrder solveLP (Ranking& ranking) {
	ranking.initiateMatrixFromFile();

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

	return LinearOrder(size);
}

LinearOrder solveILP (Ranking& ranking) {
	ranking.initiateMatrixFromFile();

	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();

	try {
		// Initialize environment and model
		IloEnv env;
		IloModel model(env);
		IloCplex cplex(model);

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
	} catch (IloException& e) {
		cout << "Exception: " << e << endl;
	}

	return LinearOrder(size);
}