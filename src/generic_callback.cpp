#include<generic_callback.h>
#include<ilp_solver.h>
#include<ranking.h>

void GenericCallback::invoke (const IloCplex::Callback::Context &context) {
	int size = x.getSize();

	// Ranking ranking(size, matrix);
	// solvePartition(ranking);

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			float c = context.getRelaxationPoint(x[i][j]);
		} 
	}
}

GenericCallback::~GenericCallback () {

}