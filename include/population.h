#ifndef POPULATION_H
#define POPULATION_H

#include<chromosome.h>
#include<random.h>
#include<io.h>

#include<vector>
#include<algorithm>

using std::vector;
using std::sort;

enum TYPE {ONE, TWO};

class Population {
	private:
		static mt19937 rng;
		static uniform_real_distribution<> dist;

		int size;
		int chromosome_length;
		vector<Chromosome> individuals;
		function<float(Chromosome)> fitnessFunc;

	public:
		static void initialize (int);

		Population (int, int);
		Population (const Population&);

		void setFitnessFunc (function<float(Chromosome)>);

		void computeFitness ();
		void sortByFitness ();
		void mutation (float);
		void crossover (float, float, float, TYPE);
		Population nextGeneration (float, float, float, TYPE);

		Chromosome getBest ();
		void print ();
};

#endif