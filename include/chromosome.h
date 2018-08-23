#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include<linear_order.h>
#include<random.h>
#include<io.h>

#include<vector>
#include<functional>

using std::vector;
using std::function;

class Gene {
	public:
		int index;
		float random_value;

		Gene () {};
		Gene (int, float);

		bool operator < (Gene);
};

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
		Chromosome (vector<int>);
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