#ifndef GENETIC_SOLVER_H
#define GENETIC_SOLVER_H

#include<ranking.h>
#include<linear_order.h>
#include<population.h>

#include<vector>

using std::vector;

class GeneticSolver {
	private:
		int population_size = 2000;
		int num_generations = 200;
		float eliteBias = 0.6;
		float eliteRatio = 0.2;
		float mutationRatio = 0.2;

		Ranking &r;

	public:
		GeneticSolver (Ranking &r): r(r) {}

		LinearOrder solve ();
		float getWeight (LinearOrder);
};

#endif