#ifndef POPULATION_H
#define POPULATION_H

#include<chromosome.h>

#include<random>
#include<vector>
#include<algorithm>
#include<iostream>

using std::mt19937;
using std::random_device;
using std::uniform_real_distribution;
using std::uniform_int_distribution;

using std::vector;

using std::sort;

using std::cout;
using std::endl;

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
		void crossover (float, float, float);
		Population nextGeneration (float, float, float);

		Chromosome getBest ();
		void print ();
};

#endif