#include<main.h>
#include<max_flow.h>

int main(int argc, char** argv) {
	Ranking ranking(argv[1]);

	// int size = 3;
	// vector<float> random{-1.0, -1.0, 3.0};
	// vector<vector<int>> partial;
	// vector<int> one{0, 0, 1};
	// vector<int> two{0, 0, 1};
	// vector<int> three{0, 0, 0};
	// partial.push_back(one);
	// partial.push_back(two);
	// partial.push_back(three);

	// auto sol = solve_max_flow(size, random, partial);
	// for (int i = 0; i < size; i++) {
	// 	cout << sol[i] << " ";
	// }
	// cout << endl;

	LinearOrder best = solveILP(ranking, false);

	// cout << "Order is: ";
	// best.printOrder();
	// cout << endl;

	// cout << "Valid: " << best.validate() << endl;
	// cout << "Weight is: " << ranking.getWeight(best) << endl;

	// solve_c(ranking.getSize(), ranking.getMatrix());
}
