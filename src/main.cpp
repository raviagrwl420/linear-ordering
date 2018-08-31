#include<main.h>

int main(int argc, char** argv) {

	// LinearOrder p1(10);
	// LinearOrder p2(10);

	// p1.printOrder();
	// p2.printOrder();

	// LinearOrder p3 = p1.crossover(p2, 0.5);

	// p3.printOrder();

	cout << argv[1] << endl;
	Ranking ranking(argv[1]);

	LinearOrder best = solvePartition(ranking);


	// GeneticSolver solver(ranking);

	// LinearOrder best = solver.solve(true);

	cout << "Order is: ";
	best.printOrder();
	cout << endl;

	cout << "Valid: " << best.validate() << endl;

	// cout << "Weight is: " << solver.getWeight(best) << endl;
	cout << "Weight is: " << ranking.getWeight(best.getOrder()) << endl;
}
