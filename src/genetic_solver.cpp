#include<genetic_solver.h>

LinearOrder GeneticSolver::solve () {
	// Params
	population_size = 2000;
	num_generations = 200;
	eliteBias = 0.5;
	eliteRatio = 0.2;
	mutationRatio = 0.2;

	// Initialize RNGs
	Population::initialize(5);
	Chromosome::initialize(6);
	LinearOrder::initialize(7);

	// Initialize the matrix
	r.initiate_matrix_from_file();

	// Chromosome length equals the size of the square matrix
	int chromosome_length = r.getSize();

	// Initialize a random population
	Population first(population_size, chromosome_length);

	// Define fitness function
	Ranking ranking = r;
	function<float(Chromosome)> fitnessFunc = [ranking] (Chromosome chromosome) -> float {
		LinearOrder order(chromosome.getGenes());
		float weight = ranking.get_weight(order.getOrder());
		return weight;
	};

	// Set the fitness function
	first.setFitnessFunc(fitnessFunc);

	// Run for num_generations
	Chromosome best;
	LinearOrder bestOrder;
	Population nextGen = first;
	for (int i = 0; i < num_generations; i++) {
		nextGen = nextGen.nextGeneration(eliteRatio, mutationRatio, eliteBias);
		best = nextGen.getBest();
		bestOrder = LinearOrder(best.getGenes());
		float weight = ranking.get_weight(bestOrder.getOrder());
		cout << "Generation " << i << " Weight: " << weight << endl;
	}

	return bestOrder;
}

float GeneticSolver::getWeight (LinearOrder order) {
	return r.get_weight(order.getOrder());
}