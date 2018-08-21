#ifndef LINEAR_ORDER_H
#define LINEAR_ORDER_H

#include<random.h>
#include<io.h>

#include<vector>
#include<algorithm>

using std::vector;
using std::sort;

class Element {
	public:
		int node;
		float val;

		Element () {};
		Element (int, float);

		bool operator < (Element);
};

class LinearOrder {
	private:
		static mt19937 rng;
		static uniform_real_distribution<> dist;
		
		int numNodes;
		vector<Element> nodes;
	
	public:
		static void initialize (int);

		LinearOrder () {};
		LinearOrder (int);
		LinearOrder (vector<float>);
		LinearOrder (vector<int>);
		LinearOrder (const LinearOrder&);

		vector<int> getOrder ();
		void printOrder ();

		int countInversions (LinearOrder&);
		LinearOrder crossover (LinearOrder&, float bias);
};

#endif