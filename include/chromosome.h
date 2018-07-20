#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include<linear_order.h>

#include<random>
#include<vector>
#include<iostream>
#include<functional>

using std::mt19937;
using std::random_device;
using std::uniform_real_distribution;

using std::vector;

using std::cout;
using std::endl;

using std::function;

class Chromosome {
	private:
		static mt19937 rng;
		static uniform_real_distribution<> dist;

		int length;
		vector<float> genes;
		float fitness = 0;

	public:
		static void initialize (int);

		Chromosome () {};
		Chromosome (int);
		Chromosome (vector<float>);
		Chromosome (const Chromosome&);

		vector<float> getGenes ();
		float getFitness ();

		void computeFitness (function<float(Chromosome)>);
		bool operator < (Chromosome);

		Chromosome crossover (Chromosome, float);
		Chromosome crossover2 (Chromosome, float);

		void print ();
};

#endif