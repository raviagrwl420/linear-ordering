#include<chromosome.h>

// Initialize static variables with default values
mt19937 Chromosome::rng = mt19937(random_device()());
uniform_real_distribution<> Chromosome::dist = uniform_real_distribution<>(0, 1);

void Chromosome::initialize (int seed) {
	rng = mt19937(seed);
}

Chromosome::Chromosome (int n) {
	length = n;

	for (int i = 0; i < length; i++) {
		genes.push_back(dist(rng));
	}
}

Chromosome::Chromosome (vector<float> genes) {
	this->length = genes.size();
	this->genes = genes;
}

Chromosome::Chromosome (const Chromosome& original) {
	length = original.length;

	for (int i = 0; i < length; i++) {
		genes.push_back(original.genes[i]);
	}
}

float Chromosome::fitness () {
	float sum = 0.0;

	for (float val : genes) {
		sum += val;
	}

	return sum;
}

bool Chromosome::operator < (Chromosome other) {
	return this->fitness() > other.fitness();
}

Chromosome Chromosome::crossover (Chromosome other, float bias) {
	vector<float> crossed;

	for (int i = 0; i < length; i++) {
		float random = dist(rng);

		if (random < bias) {
			crossed.push_back(this->genes[i]);
		} else {
			crossed.push_back(other.genes[i]);
		}
	}

	return Chromosome(crossed);
}

void Chromosome::print () {
	for (int i = 0; i < length; i++) {
		cout << genes[i] << " ";
	}
	cout << endl;
}