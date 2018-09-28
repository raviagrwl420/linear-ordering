#include<main.h>

int main(int argc, char** argv) {
	Ranking ranking(argv[1]);

	LinearOrder best = solveILP(ranking, false);

	cout << "Order is: ";
	best.printOrder();
	cout << endl;

	cout << "Valid: " << best.validate() << endl;
	cout << "Weight is: " << ranking.getWeight(best) << endl;
}
