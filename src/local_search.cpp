#include<local_search.h>

// Perform local search on the passed linear order
LinearOrder localSearch (int size, int** matrix, LinearOrder order) {
	for (int i = 0; i < size; i++) {
		int max_improvement = 0;
		int improvement = 0;
		int nodeIndex, insertIndex;

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

		if (max_improvement > 0) {
			order.applyInsertMove(nodeIndex, insertIndex);
		}
	}

	return order;
}

// Perform local search on the passed linear order
LinearOrder localSearchExpensive (int size, int** matrix, LinearOrder order) {
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

int** extractSubMatrix(int** matrix, int size, vector<int> nodes) {
	int** subMatrix = new int*[size];
	for (int i = 0; i < size; i++) {
		subMatrix[i] = new int[size];
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			subMatrix[i][j] = matrix[nodes[i]][nodes[j]];
		}
	}

	return subMatrix;
}

LinearOrder localEnumeration (Ranking& ranking, LinearOrder order, int windowSize) {
	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();
	vector<int> orderVec = order.getOrder();

	if (size <= windowSize) {
		return order;
	}

	vector<int> window;
	vector<int> nodes;
	for (int i = 0; i < windowSize; i++) {
		window.push_back(i);
		nodes.push_back(orderVec[i]);
	}

	int** subMatrix;
	vector<int> completeOrder;
	vector<int> optimalOrder(windowSize);
	for (int i = 0; i <= size - windowSize; i++) {
		sort(window.begin(), window.end());

		subMatrix = extractSubMatrix(matrix, windowSize, nodes);
		Ranking subRanking(windowSize, subMatrix);

		int max_weight = 0;
		do {
			int weight = subRanking.getWeight(window);
			if (weight > max_weight) {
				max_weight = weight;
				for (int j = 0; j < windowSize; j++) {
					optimalOrder[j] = window[j];
				}
			}
		} while (next_permutation(window.begin(), window.end()));

		completeOrder.push_back(nodes[optimalOrder[0]]);
		nodes[optimalOrder[0]] = orderVec[windowSize + i];

		freeMemory2D(subMatrix, windowSize);
	}

	for (int i = 1; i < windowSize; i++) {
		completeOrder.push_back(nodes[optimalOrder[i]]);
	}

	return LinearOrder(completeOrder);
}