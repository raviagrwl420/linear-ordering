#include<linear_order.h>

Element::Element (int n, float v) {
	node = n;
	val = v;
}

bool Element::operator < (Element other) {
	return this->val < other.val;
}

// Initialize static variables with default values
mt19937 LinearOrder::rng = mt19937(random_device()());
uniform_real_distribution<> LinearOrder::dist = uniform_real_distribution<>(0, 1);

void LinearOrder::initialize (int seed) {
	rng = mt19937(seed);
}

LinearOrder::LinearOrder (int n) {
	numNodes = n;
	
	for (int i = 0; i < numNodes; i++) {
		nodes.push_back(Element(i, dist(rng)));
	}
}

LinearOrder::LinearOrder (vector<float> randoms) {
	numNodes = randoms.size();

	for (int i = 0; i < numNodes; i++) {
		nodes.push_back(Element(i, randoms[i]));
	}
}

LinearOrder::LinearOrder (const LinearOrder& original) {
	numNodes = original.numNodes;

	for (int i = 0; i < numNodes; i++) {
		Element originalNode = original.nodes[i];

		nodes.push_back(Element(originalNode.node, originalNode.val));
	}
}

vector<int> LinearOrder::getOrder () {
	LinearOrder temp = LinearOrder(*this);

	sort(temp.nodes.begin(), temp.nodes.end());

	vector<int> order;
	
	for (Element e: temp.nodes) {
		order.push_back(e.node);
	}

	return order;
}

void LinearOrder::printOrder () {
	vector<int> order = this->getOrder();

	for (int node: order) {
		cout << node << " ";
	}

	cout << endl;
}