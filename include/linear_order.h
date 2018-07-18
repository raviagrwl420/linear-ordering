#ifndef LINEAR_ORDER_H
#define LINEAR_ORDER_H

#include<random>
#include<iostream>
#include<vector>
#include<algorithm>

using std::mt19937;
using std::random_device;
using std::uniform_real_distribution;
using std::uniform_int_distribution;

using std::vector;

using std::sort;

using std::cout;
using std::endl;

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
		LinearOrder (const LinearOrder&);

		vector<int> getOrder ();
		void printOrder ();
};

#endif