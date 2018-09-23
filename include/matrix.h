#ifndef MATRIX_H
#define MATRIX_H

template <class T, class U>
T** extractSubMatrix (T** matrix, int size, U nodes) {
	T** newMatrix = new T*[size];
	for (int i = 0; i < size; i++) {
		newMatrix[i] = new T[size];
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			newMatrix[i][j] = matrix[nodes[i]][nodes[j]];
		}
	}

	return newMatrix;
}

#endif