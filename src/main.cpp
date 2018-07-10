#include<main.h>

int main() {
	Chromosome::initialize(1);
	Population::initialize(2);

	Population first(100, 10);

	first.print();

	for (int i = 0; i < 1000; i++) {
		first = first.nextGeneration();
		first.sortByFitness();
		first.print();
	}

	// Chromosome a(10);
	// Chromosome b(10);

	// a.print();
	// b.print();

	// float fitness = a.fitness();
	// cout << fitness << endl;

	// Chromosome c = a.crossover(b, 0.5);

	// c.print();
}
