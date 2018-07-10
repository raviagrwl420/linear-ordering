#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include<random>
#include<vector>
#include<iostream>

using std::mt19937;
using std::random_device;
using std::uniform_real_distribution;

using std::vector;

using std::cout;
using std::endl;

class Chromosome {
	private:
		static mt19937 rng;
		static uniform_real_distribution<> dist;

		int length;
		vector<float> genes;

	public:
		static void initialize (int);

		Chromosome (int);
		Chromosome (vector<float>);
		Chromosome (const Chromosome&);

		float fitness ();
		bool operator < (Chromosome);

		Chromosome crossover (Chromosome, float);

		void print ();
};

#endif