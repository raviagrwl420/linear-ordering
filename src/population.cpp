#include<population.h>

// Initialize static variables with default values
mt19937 Population::rng = mt19937(random_device()());
uniform_real_distribution<> Population::dist = uniform_real_distribution<>(0, 1);

// Initialize the random number generator
void Population::initialize (int seed) {
	rng = mt19937(seed);
}

// Initialize a population of size m with chromosomes of length n
Population::Population (int m, int n) {
	size = m;
	chromosome_length = n;

	for (int i = 0; i < size; i++) {
		individuals.push_back(Chromosome(chromosome_length));
	}
}

// Copy constructor
Population::Population (const Population& original) {
	size = original.size;
	chromosome_length = original.chromosome_length;
	fitnessFunc = original.fitnessFunc;

	for (int i = 0; i < size; i++) {
		individuals.push_back(Chromosome(original.individuals[i]));
	}
}

// Setter for fitnessFunc
void Population::setFitnessFunc (function<float(Chromosome)> f) {
	fitnessFunc = f;
}

// Compute fitness for the population
void Population::computeFitness () {
	for (Chromosome &individual: individuals) {
		individual.computeFitness(fitnessFunc);
	}
}

// Sort the population by fitness values
void Population::sortByFitness () {
	// Compute fitness for all chromosomes
	this->computeFitness();

	// Sort using the sort function which uses the fitness values
	sort(individuals.begin(), individuals.end());
}

// Add mutation
void Population::mutation (float mutationRatio) {
	int numMutation = mutationRatio * size;

	for (int i = 1; i <= numMutation; i++) {
		individuals[size - i] = Chromosome(chromosome_length);
	}
}

// Generate new population by crossover
void Population::crossover (float eliteRatio, float mutationRatio, float eliteBias, TYPE type) {
	int numElite = eliteRatio * size;
	int numMutation = mutationRatio * size;
	int numCrossover = size - numElite - numMutation;

	uniform_int_distribution<> eliteDist = uniform_int_distribution<>(0, numElite-1);
	uniform_int_distribution<> crossoverDist = uniform_int_distribution<>(0, size - 1);

	vector<Chromosome> crossed;
	for (int i = 1; i <= numCrossover; i++) {
		int eliteIndex = eliteDist(rng);
		int crossoverIndex = crossoverDist(rng);

		if (type == ONE) {
			crossed.push_back(individuals[eliteIndex].crossover(individuals[crossoverIndex], eliteBias));	
		} else {
			crossed.push_back(individuals[eliteIndex].crossover2(individuals[crossoverIndex], eliteBias));
		}
	}

	for (int i = 0; i < numCrossover; i++) {
		individuals[numElite + i] = crossed[i];
	}
}

// Generate the population for the next generation
Population Population::nextGeneration (float eliteRatio, float mutationRatio, float eliteBias, TYPE type) {
	// Sort initial population
	this->sortByFitness();

	// Make a copy
	Population copy = Population(*this);

	// Add random solutions at the end
	copy.mutation(mutationRatio);
	copy.crossover(eliteRatio, mutationRatio, eliteBias, type);

	return copy;
}

Chromosome Population::getBest () {
	this->sortByFitness();
	return individuals[0];
}

// Print population
void Population::print () {
	cout << "Fitness: " << individuals[0].getFitness() << endl << endl;
}
