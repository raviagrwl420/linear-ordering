#include<chromosome.h>

// Initialize static variables with default values
mt19937 Chromosome::rng = mt19937(random_device()());
uniform_real_distribution<> Chromosome::dist = uniform_real_distribution<>(0, 1);

// Initialize the random number generator
void Chromosome::initialize (int seed) {
	rng = mt19937(seed);
}

// Create a new chromosome of length n with random genes with values [0, 1]
Chromosome::Chromosome (int n) {
	length = n;

	for (int i = 0; i < length; i++) {
		float gene = dist(rng);
		genes.push_back(gene);
	}
}

// Create a new chromosome with the genes passed as argument
Chromosome::Chromosome (vector<float> genes) {
	this->length = genes.size();
	this->genes = genes;
}

// Copy constructor
Chromosome::Chromosome (const Chromosome& original) {
	length = original.length;

	for (int i = 0; i < length; i++) {
		genes.push_back(original.genes[i]);
	}
}

// Compute fitness
float Chromosome::fitness () {
	float sum = 0.0;

	for (float val : genes) {
		sum += val;
	}

	return sum;
}

// Implements the `<` operator for sorting
bool Chromosome::operator < (Chromosome other) {
	return this->fitness() > other.fitness();
}

// Basic crossover operator
Chromosome Chromosome::crossover (Chromosome other, float crossoverBias) {
	vector<float> crossed;

	for (int i = 0; i < length; i++) {
		float random = dist(rng);

		if (random < crossoverBias) {
			crossed.push_back(this->genes[i]);
		} else {
			crossed.push_back(other.genes[i]);
		}
	}

	return Chromosome(crossed);
}

// Print a chromosome
void Chromosome::print () {
	for (int i = 0; i < length; i++) {
		cout << genes[i] << " ";
	}
	cout << endl;
}