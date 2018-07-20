#ifndef GENETIC_SOLVER_H
#define GENETIC_SOLVER_H

#include<ranking.h>
#include<linear_order.h>
#include<population.h>

#include<vector>

using std::vector;

class GeneticSolver {
	private:
		int population_size = 100;
		int num_generations = 10;
		float eliteBias = 0.5;
		float eliteRatio = 0.2;
		float mutationRatio = 0.2;

		Ranking &r;

	public:
		GeneticSolver (Ranking &r): r(r) {}

		LinearOrder solve ();
		float getWeight (LinearOrder);
};

#endif