#include<local_search.h>

// Perform local search on the passed linear order
LinearOrder localSearch (Ranking& ranking, LinearOrder order) {
	ranking.initiateMatrixFromFile();

	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();

	int max_improvement = 0;
	do {
		int improvement = 0;
		int nodeIndex, insertIndex;

		max_improvement = 0;
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (i != j) {
					improvement = order.improvementForInsertMove(matrix, i, j);

					if (improvement > max_improvement) {
						max_improvement = improvement;
						nodeIndex = i;
						insertIndex = j;
					}

				}
			}
		}

		if (max_improvement > 0) {
			order.applyInsertMove(nodeIndex, insertIndex);
		}
	} while (max_improvement > 0);

	return order;
}