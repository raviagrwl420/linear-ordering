#ifndef MEMORY_H
#define MEMORY_H

template <class T>
void freeMemory (T vars) {
	int size = vars.getSize();
	for (int i = 0; i < size; i++) {
		vars[i].end();
	}
	vars.end();
}

template <class T>
void freeMemory1D(T* array1d, int size) {
	delete [] array1d;
};

template <class T>
void freeMemory2D(T** array2d, int size) {
	for (int i = 0; i < size; i++) {
		delete [] array2d[i];
	}
	delete [] array2d;
};

template <class T>
void freeMemory3D(T*** array3d, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			delete [] array3d[i][j];
		}
		delete [] array3d[i];
	}
	delete [] array3d;
};

#endif