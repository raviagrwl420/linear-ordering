#include<generic_callback.h>
#include<ilp_solver.h>
#include<ranking.h>

void GenericCallback::invoke (const IloCplex::Callback::Context &context) {
	int size = x.getSize();

	Ranking ranking(size, matrix);
	solvePartition(ranking);
}

GenericCallback::~GenericCallback () {

}