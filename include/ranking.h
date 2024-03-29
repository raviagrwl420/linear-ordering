#ifndef RANKING_H
#define RANKING_H

#include<linear_order.h>
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
		int *potentials;
		ifstream inputFile;

	public:
		Ranking (const Ranking&);
		Ranking (const char*);
		Ranking (int size, int** matrix);

		int getSize () const;

		void initiateMatrixFromFile ();
		int **getMatrix () const;
		void printMatrix () const;

		void computePotentials ();
		int *getPotentials () const;

		int getWeight (vector<int>) const;
		int getWeight (LinearOrder) const;
};

#endif