#include<main.h>

int main() {
	Ranking ranking("./data/N-atp134");

	GeneticSolver solver(ranking);

	LinearOrder best = solver.solve();

	cout << "Order is: ";
	best.printOrder();

	cout << endl;

	cout << "Weight is: " << solver.getWeight(best) << endl;
}
