#include<ranking.h>

vector<string> getTokens (string s, string delim) {
	vector<string> tokens;

	istringstream iss(s);
	copy(istream_iterator<string>(iss),
		istream_iterator<string>(),
		back_inserter(tokens));
	
	return tokens;
}

void Ranking::initiate_matrix_from_file () {
	string line;

	getline(inputFile, line);
	numNodes = stoi(line);

	m = new int*[numNodes];

	for (int i = 0; i < numNodes; i++) {
		m[i] = new int[numNodes];
	}

	int i = 0;
	while(getline(inputFile, line)) {
		vector<string> tokens = getTokens(line, " ");

		int j = 0;
		for (string token: tokens) {
			int m_ij = stoi(token);
			m[i][j++] = m_ij;
		}

		i++;
	}
}

void Ranking::print_matrix () {
	for (int i = 0; i < numNodes; i++) {
		for (int j = 0; j < numNodes; j++) {
			cout << m[i][j] << " ";
		}
		cout << endl;
	}
}

int Ranking::get_weight (int order[]) {
	int weight = 0;
	for (int i = 0; i < numNodes; i++) {
		for (int j = i + 1; j < numNodes; j++) {
			weight += m[order[i]][order[j]];
		}
	}
	return weight;
}