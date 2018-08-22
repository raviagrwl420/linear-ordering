#ifndef LINEAR_ORDER_H
#define LINEAR_ORDER_H

#include<random.h>
#include<io.h>

#include<vector>
#include<algorithm>

using std::vector;
using std::sort;

class Node {
	public:
		int index;
		float random_value;

		Node () {};
		Node (int, float);

		bool operator < (Node);
};

class LinearOrder {
	private:
		static mt19937 rng;
		static uniform_real_distribution<> dist;
		
		int size;
		vector<int> order;
		vector<Node> nodes;
	
	public:
		static void initialize (int);

		LinearOrder () {};
		LinearOrder (int);
		LinearOrder (vector<float>);
		LinearOrder (vector<int>);
		LinearOrder (const LinearOrder&);

		vector<int> getPermutation (vector<float> randoms);
		vector<int> getOrder ();
		void printOrder ();

		int improvementForInsertMove(int**, int, int);
		void applyInsertMove (int, int);

		int countInversions (LinearOrder&);
		LinearOrder crossover (LinearOrder&, float bias);
};

#endif