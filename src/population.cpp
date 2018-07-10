#include<population.h>

// Initialize static variables with default values
mt19937 Population::rng = mt19937(random_device()());
uniform_real_distribution<> Population::dist = uniform_real_distribution<>(0, 1);

void Population::initialize (int seed) {
	rng = mt19937(seed);
}

Population::Population (int m, int n) {
	size = m;
	chromosome_length = n;

	for (int i = 0; i < size; i++) {
		individuals.push_back(Chromosome(chromosome_length));
	}
}

Population::Population (const Population& original) {
	size = original.size;
	chromosome_length = original.chromosome_length;

	for (int i = 0; i < size; i++) {
		individuals.push_back(Chromosome(original.individuals[i]));
	}
}

void Population::sortByFitness () {
	sort(individuals.begin(), individuals.end());
}

void Population::addRandom (float ratio) {
	int numRandom = ratio * size;

	for (int i = 1; i <= numRandom; i++) {
		individuals[size - i] = Chromosome(chromosome_length);
	}
}

void Population::crossover (float eliteRatio, float randomRatio) {
	int numElite = eliteRatio * size;
	int numRandom = randomRatio * size;
	int numCrossover = size - numElite - numRandom;

	uniform_int_distribution<> eliteDist = uniform_int_distribution<>(0, numElite-1);
	uniform_int_distribution<> crossoverDist = uniform_int_distribution<>(numElite, numElite + numCrossover - 1);

	vector<Chromosome> crossed;
	for (int i = 1; i <= numCrossover; i++) {
		int eliteIndex = eliteDist(rng);
		int crossoverIndex = crossoverDist(rng);

		crossed.push_back(individuals[eliteIndex].crossover(individuals[crossoverIndex], 0.7));
	}

	for (int i = 0; i < numCrossover; i++) {
		individuals[numElite + i] = crossed[i];
	}
}

Population Population::nextGeneration () {
	// Sort initial population
	this->sortByFitness();

	// Make a copy
	Population copy = Population(*this);

	// Add random solutions at the end
	copy.addRandom(0.2);
	copy.crossover(0.2, 0.2);

	return copy;
}

void Population::print () {
	cout << "Fitness: " << individuals[0].fitness() << endl << endl;
}
