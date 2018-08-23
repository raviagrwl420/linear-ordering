#include<main.h>

int main() {

	// LinearOrder p1(10);
	// LinearOrder p2(10);

	// p1.printOrder();
	// p2.printOrder();

	// LinearOrder p3 = p1.crossover(p2, 0.5);

	// p3.printOrder();

	Ranking ranking("./data/N-atp134");

	GeneticSolver solver(ranking);

	LinearOrder best = solver.solve(true);

	cout << "Order is: ";
	best.printOrder();

	cout << endl;

	cout << "Weight is: " << solver.getWeight(best) << endl;
}
