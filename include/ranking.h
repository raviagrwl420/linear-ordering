#ifndef RANKING_H
#define RANKING_H

#include<vector>
#include<string>
#include<sstream>
#include<algorithm>
#include<iterator>

#include<fstream>
#include<iostream>

using std::vector;
using std::string;
using std::stoi;
using std::stringstream;
using std::istringstream;
using std::istream_iterator;
using std::back_inserter;
using std::copy;

using std::ifstream;

using std::cout;
using std::endl;

class Ranking {
	private:
		int numNodes;
		int **m;
		ifstream inputFile;

	public:
		Ranking (const char* s): inputFile(s) {};

		void initiate_matrix_from_file ();
		void print_matrix ();

		int get_weight (int order[]);
};

#endif