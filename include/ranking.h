#ifndef RANKING_H
#define RANKING_H

#include<io.h>

#include<vector>
#include<string>
#include<sstream>
#include<algorithm>
#include<iterator>

#include<fstream>

using std::vector;
using std::string;
using std::stoi;
using std::stringstream;
using std::istringstream;
using std::istream_iterator;
using std::back_inserter;
using std::copy;

using std::ifstream;

class Ranking {
	private:
		int size;
		int **matrix;
		ifstream inputFile;

	public:
		Ranking (const Ranking&);
		Ranking (const char* s): inputFile(s) {};

		int getSize ();

		void initiateMatrixFromFile ();
		int **getMatrix () const;
		void printMatrix () const;

		int getWeight (vector<int>) const;
};

#endif