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

	sort(nodes.begin(), nodes.end());
}

LinearOrder::LinearOrder (vector<float> randoms) {
	numNodes = randoms.size();

	for (int i = 0; i < numNodes; i++) {
		nodes.push_back(Element(i, randoms[i]));
	}

	sort(nodes.begin(), nodes.end());
}

LinearOrder::LinearOrder (const LinearOrder& original) {
	numNodes = original.numNodes;

	for (int i = 0; i < numNodes; i++) {
		Element originalNode = original.nodes[i];

		nodes.push_back(Element(originalNode.node, originalNode.val));
	}
}

vector<int> LinearOrder::getOrder () {
	vector<int> order;
	
	for (Element e: nodes) {
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

int LinearOrder::countInversions (LinearOrder& other) {
	vector<int> originalOrder = this->getOrder();
	vector<int> otherOrder = other.getOrder();

	int *map = new int[numNodes];
	for (int i = 0; i < numNodes; i++) {
		map[originalOrder[i]] = i;
	}

	int *mapped = new int[numNodes];
	for (int i = 0; i < numNodes; i++) {
		mapped[i] = map[otherOrder[i]];
	}

	int inversions = 0;
	for (int i = 0; i < numNodes; i++) {
		for (int j = i + 1; j < numNodes; j++) {
			if (mapped[j] < mapped[i]) {
				inversions += 1;
			}
		}
	}

	return inversions;
}

LinearOrder LinearOrder::crossover (LinearOrder& other, float bias) {

	// cout << "This! ";
	// this->printOrder();
	// cout << endl;

	// cout << "Other! ";
	// other.printOrder();
	// cout << endl;

	vector<int> originalOrder = this->getOrder();
	vector<int> otherOrder = other.getOrder();

	int *map = new int[numNodes];
	for (int i = 0; i < numNodes; i++) {
		map[originalOrder[i]] = i;
	}

	int *mapped = new int[numNodes];
	for (int i = 0; i < numNodes; i++) {
		mapped[i] = map[otherOrder[i]];
	}

	for (int i = 0; i < numNodes; i++) {
		for (int j = 1; j < numNodes; j++) {
			if (mapped[j-1] < mapped[j]) {
				continue;
			}

			float random = dist(rng);
			if (random < bias) {
				int temp;

				temp = mapped[j-1];
				mapped[j-1] = mapped[j];
				mapped[j] = temp;

				temp = otherOrder[j-1];
				otherOrder[j-1] = otherOrder[j];
				otherOrder[j] = temp;
			}
		}
	}

	float *randoms = new float[numNodes];
	for (int i = 0; i < numNodes; i++) {
		randoms[otherOrder[i]] = (1.0 * i) / numNodes;
	}

	vector<float> randomsVec;
	for (int i = 0; i < numNodes; i++) {
		randomsVec.push_back(randoms[i]);
	}

	LinearOrder computed(randomsVec);

	// cout << "Computed! ";
	// computed.printOrder();
	// cout << endl;

	return computed;
}