#include<ilp_solver.h>
#include<legacy_callback.h>
#include<max_flow.h>

#include<boost/timer/timer.hpp>

#define EPS 1e-8
#define MIP_START_MIN_SIZE 35
#define ILP_MAX_SIZE 35

// Solve LP
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

		// Extract solution
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				fractionals[i][j] = cplex.getValue(x[i][j]);
			}
		}

		// Free Memory
		freeMemory(x);
		obj.end();
		cplex.end();
		model.end();
		env.end();

		return fractionals;
	} catch (IloException& e) {
		cout << "SolveLP Exception" << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}
}

void setCplexParams (IloCplex cplex) {
	// cplex.setParam(IloCplex::Param::TimeLimit, 600);
	cplex.setParam(IloCplex::Param::MIP::Tolerances::Integrality, EPS);
	cplex.setParam(IloCplex::Param::Simplex::Tolerances::Feasibility, EPS);
	cplex.setParam(IloCplex::Param::MIP::Strategy::Search, 1);
	cplex.setParam(IloCplex::Param::MIP::Strategy::VariableSelect, 1);
	// cplex.setParam(IloCplex::Param::MIP::Strategy::NodeSelect, 3);
	// cplex.setParam(IloCplex::Param::Parallel, 1);
	// cplex.setParam(IloCplex::Param::Threads, 3);
	// cplex.setParam(IloCplex::Param::MIP::Strategy::HeuristicFreq, 5);
}

void setMandatoryCallbacks (Ranking& ranking, IloCplex cplex, IloEnv env, IloArray<IloBoolVarArray> x, IloObjective obj, vector<Constraint> constraints) {
	// Add callback using generic callback
	// HeuristicCallback cb(x, matrix);
	// cplex.use(&cb, IloCplex::Callback::Context::Id::Relaxation);

	// Add callback using legacy callback
	cplex.use(LegacyLazyConstraintCallback(env, cplex, x, obj, ranking));
	cplex.use(LegacyUserCutCallback(env, cplex, x, obj, ranking, constraints));
}

void setOptionalCallbacks (Ranking& ranking, IloCplex cplex, IloEnv env, IloArray<IloBoolVarArray> x, IloObjective obj, vector<Constraint> constraints) {
	cplex.use(LegacyHeuristicCallback(env, cplex, x, obj, ranking));

	// cplex.use(LegacyBranchCallback(env, cplex, x, ranking));
	// cplex.use(LegacyNodeCallback(env));
}

// Add MIP Start
void addMIPStart (Ranking& ranking, IloCplex cplex, IloEnv env, IloArray<IloBoolVarArray> x) {
	int size = ranking.getSize();

	if (size > MIP_START_MIN_SIZE) {
		// Flatten x for MIP start
		IloNumVarArray flattenedX(env, size * size);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				flattenedX[i * size + j] = x[i][j];
			}
		}
	
		IloNumArray startSolution(env, size * size);

		LinearOrder startOrder = solvePartition(ranking, NULL);
		for (int i = 0; i < size; i++) {
			for (int j = i + 1; j < size; j++) {
				startSolution[startOrder[i] * size + startOrder[j]] = 1;
			}
		}

		cplex.addMIPStart(flattenedX, startSolution);

		// Free Memory
		flattenedX.end();
		startSolution.end();
	}
}

// Add 3-cycle constraints
void addCycleConstraints (Ranking& ranking, IloCplex cplex, IloEnv env, IloArray<IloBoolVarArray> x, vector<Constraint> constraints) {
	IloConstraintArray cycleConstraints;
	for (Constraint constraint: constraints) {
		int i = constraint.i;
		int j = constraint.j;
		int k = constraint.k;

		cycleConstraints.add(x[i][j] + x[j][k] + x[k][i] <= 2);
	}
	cplex.addLazyConstraints(cycleConstraints);
	// cplex.addUserCuts(cycleConstraints);
}

// Decode the solution
LinearOrder decodeSolution (Ranking& ranking, IloCplex cplex, IloArray<IloBoolVarArray> x) {
	int size = ranking.getSize();

	// Add the values for each row
	int* totals = new int[size];
	for (int i = 0; i < size; i++) {
		int total = 0;
		for (int j = 0; j < size; j++) {
			double value = cplex.getValue(x[i][j]);
			total += (int) round(value); // Rounding is important!
		}
		totals[i] = total;
	}

	// Insert the node in its proper position in the order
	vector<int> orderVec(size);
	for (int i = 0; i < size; i++) {
		int index = (size - 1) - totals[i];
		orderVec[index] = i;
	}

	// Free Memory
	delete [] totals;

	return LinearOrder(orderVec);
}

// Initialize constraints
vector<Constraint> initializeConstraints (Ranking& ranking) {
	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();

	vector<Constraint> constraints;
	// constraints.reserve(size*(size-1)*(size-2));
	for (int i = 0; i < size; i++) {
		for (int j = i + 1; j < size; j++) {
			for (int k = i + 1; k < size; k++) {
				if (j != k) {
					double value = matrix[i][j] + matrix[j][k] + matrix[k][i];
					constraints.push_back(Constraint(i, j, k, value));
				}
			}
		}
	}

	sort(constraints.begin(), constraints.end());

	return constraints;
}

void freeConstraints (vector<Constraint> constraints) {
	constraints.clear();
	vector<Constraint>().swap(constraints);
}

LinearOrder solveILP (Ranking& ranking, bool disableOutput) {
	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();
	
	vector<Constraint> constraints = initializeConstraints(ranking);

	try {
		// Initialize environment and model
		IloEnv env;
		IloModel model(env);
		IloCplex cplex(model);

		// Disable output TODO: Fix disabling of output
		if (disableOutput) {
			cplex.setOut(env.getNullStream());
		}

		// Initialize variable array
		IloArray<IloBoolVarArray> x(env, size);
		for (int i = 0; i < size; i++) {
			x[i] = IloBoolVarArray(env, size);
		}

		// Initialize objective function
		IloExpr objFunc(env);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					objFunc += (matrix[i][j] * x[i][j]);
				}
			}
		}
		IloObjective obj = IloMaximize(env, objFunc);
		model.add(obj);

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

		// Add cycle constraints when using generic callback
		// addCycleConstraints(ranking, cplex, env, x, constraints);

		// Set CPLEX Params
		setCplexParams(cplex);

		// Set callbacks
		setMandatoryCallbacks(ranking, cplex, env, x, obj, constraints);

		if (!disableOutput) {
			setOptionalCallbacks(ranking, cplex, env, x, obj, constraints);
		}

		// Add MIP Start
		// addMIPStart(ranking, cplex, env, x);

		// Solve
		cplex.solve();

		LinearOrder order = decodeSolution(ranking, cplex, x);

		// Free Memory
		freeMemory(x);
		objFunc.end();
		obj.end();
		cplex.end();
		model.end();
		env.end();
		freeConstraints(constraints);

		return order;
	} catch (IloException& e) {
		cout << "SolveILP Exception: " << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}
}

LinearOrder solveRecursive(int size, int** partitionMatrix, int** partialSolution) {
	Ranking ranking(size, partitionMatrix);

	LinearOrder solved;
	if (size == 1) {
		vector<int> orderVec = {0};
		solved = LinearOrder(orderVec);
	} else if (size < ILP_MAX_SIZE) {
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

		boost::timer::cpu_timer t;
		t.start();

		IloEnv env;
		IloModel model(env);
		IloCplex cplex(model);

		cplex.setOut(env.getNullStream());

		// Initialize variable array
		IloBoolVarArray x(env, size);

		// Add the variable to model to force its extraction
		model.add(x);

		// Initialize variable array
		// IloArray<IloBoolVarArray> y(env, size);
		// for (int i = 0; i < size; i++) {
		// 	y[i] = IloBoolVarArray(env, size);
		// }

		// Initialize objective function
		IloExpr obj(env);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					// if (partialSolution == NULL)
						obj += (matrix[i][j] * (x[i] - x[j]));
					// else
					// 	obj += (partialSolution[i][j] * (x[i] - x[j]));	
					// obj += (matrix[i][j] * y[i][j]);
				}
			}
		}
		model.add(IloMaximize(env, obj));

		// Add constraints
		// for (int i = 0; i < size; i++) {
		// 	for (int j = 0; j < size; j++) {
		// 		if (i != j) {
		// 			model.add(y[i][j] <= x[i]);
		// 			model.add(y[i][j] <= 1 - x[j]);	
		// 		}
		// 	}
		// }

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

		cout << "CPLEX Time Elapsed!! " << boost::timer::format(t.elapsed()) << endl;
		cout << "CPLEX OPT!! " << cplex.getObjValue() << endl;

		t.start();
		vector<int> part = solve_max_flow(ranking, partialSolution);
		cout << "Flow Time Elapsed!! " << boost::timer::format(t.elapsed()) << endl;

		int sum = 0;
		int* weights = ranking.getPotentials();
		for (int i = 0; i < size; i++) {
			if (part[i] == 1) {
				sum += weights[i];
			}
		}
		cout << "Flow OPT!! " << sum << endl;

		if (cplex.getObjValue() != sum) {
			for (int i = 0; i < size; i++) {
				cout << std::setw(4) << i << " ";
			}
			cout << endl;

			cout << "Potentials!" << endl;
			for (int i = 0; i < size; i++) {
				cout << std::setw(4) << weights[i] << " ";
			}
			cout << endl;

			cout << "CPLEX Solution!" << endl;
			for (int i = 0; i < size; i++) {
				cout << std::setw(4) << cplex.getValue(x[i]) << " ";
			}
			cout << endl;

			cout << "Flow Solution!" << endl;
			for (int i = 0; i < size; i++) {
				cout << std::setw(4) << part[i] << " ";
			}
			cout << endl;

			cout << "Partial Solution!" << endl;
			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) {
					cout << std::setw(4) << partialSolution[i][j] << " ";
				}
				cout << std::setw(4) << i << endl;
			}

			exit(EXIT_FAILURE);
		}

		// Decode solution
		// Count number of nodes in partition
		int count = 0;
		int* values = new int[size];
		for (int i = 0; i < size; i++) {
			double value = cplex.getValue(x[i]);
			values[i] = (int) round(value);
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

		// Free Memory
		freeMemory2D(partition1Matrix, size1);
		freeMemory2D(partition2Matrix, size2);
		freeMemory2D(partialSolution1Matrix, size1);
		freeMemory2D(partialSolution2Matrix, size2);
		freeMemory1D(values, size);
		freeMemory1D(partition1, size1);
		freeMemory1D(partition2, size2);
		env.end();

		return LinearOrder(finalOrder);
	} catch (IloException& e) {
		cout << "SolvePartition Exception: " << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}
}

LinearOrder solveRecursive(int size, float** partitionMatrix, int** intMatrix) {
	Ranking newRanking(size, intMatrix);

	LinearOrder solved;
	if (size < ILP_MAX_SIZE) {
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

		cplex.setOut(env.getNullStream());

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

		// Decode solution
		// Count number of nodes in partition
		int count = 0;
		int* values = new int[size];
		for (int i = 0; i < size; i++) {
			double value = cplex.getValue(x[i]);
			values[i] = (int) round(value);
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

		// Free Memory
		x.end();
		obj.end();
		cplex.end();
		model.end();
		env.end();
		freeMemory2D(partition1Matrix, size1);
		freeMemory2D(partition2Matrix, size2);
		freeMemory2D(intPartitionMatrix1, size1);
		freeMemory2D(intPartitionMatrix2, size2);
		freeMemory1D(values, size);
		freeMemory1D(partition1, size1);
		freeMemory1D(partition2, size2);

		return LinearOrder(finalOrder);
	} catch (IloException& e) {
		cout << "SolvePartition Exception: " << e.getMessage() << endl;
		e.end();
	} catch (...) {
		cout << "GenericException!" << endl;
	}
}