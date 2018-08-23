#include<genetic_solver.h>

LinearOrder recursiveRefinement (Ranking& ranking, LinearOrder order) {
	int partitions = 2;
	int minSize = 50;

	int size = ranking.getSize();

	if (size < minSize) {
		return order;
	}

	int** matrix = ranking.getMatrix();

	int partitionSize = size / partitions;
	int partitionGap = size % partitions;

	vector<int> orderVec = order.getOrder();

	vector<int> newOrder;
	for (int i = 0; i < partitions; i++) {
		cout << "i: " << i << endl;
		vector<int> partition;
		for (int j = i * partitionSize; j < (i + 1) * partitionSize; j++) {
			partition.push_back(orderVec[j]);
		}

		int** partialMatrix = new int*[partitionSize];
		for (int j = 0; j < partitionSize; j++) {
			partialMatrix[j] = new int[partitionSize];
		}

		for (int j = 0; j < partitionSize; j++) {
			for (int k = 0; k < partitionSize; k++) {
				partialMatrix[j][k] = matrix[partition[j]][partition[k]];
			}
		}

		Ranking newRanking(partitionSize, partialMatrix);

		GeneticSolver recursiveSolver(newRanking);

		LinearOrder partialOrder = recursiveSolver.solve(false);

		vector<int> partialOrderVec = partialOrder.getOrder();

		for (int j = 0; j < partitionSize; j++) {
			newOrder.push_back(partition[partialOrderVec[j]]);
		}
	}

	for (int i = partitions * partitionSize; i < size; i++) {
		newOrder.push_back(orderVec[i]);
	}

	LinearOrder newestOrder = LinearOrder(newOrder);

	return newestOrder;
}

LinearOrder GeneticSolver::solve (bool file) {
	cout << "Solver!" << endl;
	// Params
	population_size = 100;
	num_generations = 100;
	eliteBias = 0.5;
	eliteRatio = 0.2;
	mutationRatio = 0.2;

	// Initialize RNGs
	Population::initialize(5);
	Chromosome::initialize(6);
	LinearOrder::initialize(7);

	// Initialize the matrix
	if (file) {
		r.initiateMatrixFromFile();	
	}
	
	int** matrix = r.getMatrix();

	// Chromosome length equals the size of the square matrix
	int chromosome_length = r.getSize();

	// Initialize a random population
	Population first(population_size, chromosome_length);

	// Define fitness function
	Ranking ranking = r;
	function<float(Chromosome)> fitnessFunc = [ranking] (Chromosome chromosome) -> float {
		LinearOrder order(chromosome.getGenes());
		float weight = ranking.getWeight(order.getOrder());
		return weight;
	};

	function<LinearOrder(Chromosome)> localSearchProcedure = [chromosome_length, matrix] (Chromosome chromosome) -> LinearOrder {
		LinearOrder order(chromosome.getGenes());
		order = localSearch(chromosome_length, matrix, order);
		return order;
	};

	// Set the fitness function
	first.setFitnessFunc(fitnessFunc);

	// Run for num_generations
	Chromosome best;
	LinearOrder bestOrder;
	Population nextGen = first;
	for (int i = 0; i < num_generations; i++) {
		nextGen.performLocalSearch(localSearchProcedure);
		nextGen = nextGen.nextGeneration(eliteRatio, mutationRatio, eliteBias, ONE);
		best = nextGen.getBest();
		bestOrder = LinearOrder(best.getGenes());
		float weight = ranking.getWeight(bestOrder.getOrder());
		cout << "Generation " << i << " Weight: " << weight << endl;
	}

	LinearOrder newOrder = recursiveRefinement(ranking, bestOrder);

	return newOrder;
}

float GeneticSolver::getWeight (LinearOrder order) {
	return r.getWeight(order.getOrder());
}