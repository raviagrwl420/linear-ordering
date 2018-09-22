#include<ranking.h>

vector<string> getTokens (string s, string delim) {
	vector<string> tokens;

	istringstream iss(s);
	copy(istream_iterator<string>(iss),
		istream_iterator<string>(),
		back_inserter(tokens));
	
	return tokens;
}

Ranking::Ranking (const Ranking& original) {
	size = original.size;
	matrix = original.matrix;
}

Ranking::Ranking (const char* s) {
	inputFile = ifstream(s);
	initiateMatrixFromFile();
}

int Ranking::getSize () const {
	return size;
}

void Ranking::initiateMatrixFromFile () {
	string line;

	getline(inputFile, line);
	size = stoi(line);

	matrix = new int*[size];

	for (int i = 0; i < size; i++) {
		matrix[i] = new int[size];
	}

	int i = 0;
	while(getline(inputFile, line)) {
		vector<string> tokens = getTokens(line, " ");

		int j = 0;
		for (string token: tokens) {
			int m_ij = stoi(token);
			matrix[i][j++] = m_ij;
		}

		i++;
	}
}

int **Ranking::getMatrix () const {
	return matrix;
}

void Ranking::printMatrix () const {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			cout << matrix[i][j] << " ";
		}
		cout << endl;
	}
}

int Ranking::getWeight (vector<int> order) const {
	int weight = 0;
	for (int i = 0; i < size; i++) {
		for (int j = i + 1; j < size; j++) {
			weight += matrix[order[i]][order[j]];
		}
	}
	return weight;
}

int Ranking::getWeight (LinearOrder order) const {
	int weight = 0;
	for (int i = 0; i < size; i++) {
		for (int j = i + 1; j < size; j++) {
			weight += matrix[order[i]][order[j]];
		}
	}
	return weight;
}