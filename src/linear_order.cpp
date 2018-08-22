#include<linear_order.h>

// Create a new node with index i and random value v
Node::Node (int i, float v) {
	index = i;
	random_value = v;
}

// Implements the `<` operator for sorting
bool Node::operator < (Node other) {
	return this->random_value < other.random_value;
}

// Initialize static variables with default values
// Initialize a default random number generator
mt19937 LinearOrder::rng = mt19937(random_device()());

// Initialize a uniform random distribution [0, 1]
uniform_real_distribution<> LinearOrder::dist = uniform_real_distribution<>(0, 1);

// Initialize a random number generator with a seed
void LinearOrder::initialize (int seed) {
	rng = mt19937(seed);
}

// Create a permutaion (i.e. a permutation of [1, 2, ... , n]) from a vector of randoms
vector<int> LinearOrder::getPermutation (vector<float> randoms) {
	int size = randoms.size();

	vector<Node> nodes;
	for (int i = 0; i < size; i++) {
		nodes.push_back(Node(i, randoms[i]));
	}
	sort(nodes.begin(), nodes.end());

	vector<int> permutation;
	for (Node node: nodes) {
		permutation.push_back(node.index);
	}

	return permutation;
}

// Create a new random linear order of size n.
LinearOrder::LinearOrder (int n) {
	size = n;
	
	vector<float> randoms;
	for (int i = 0; i < size; i++) {
		randoms.push_back(dist(rng));
	}

	order = getPermutation(randoms);
}

// Create a new linear order from a vector of randoms
LinearOrder::LinearOrder (vector<float> randoms) {
	size = randoms.size();

	order = getPermutation(randoms);
}

// Create a new linear order from a vector of ints
LinearOrder::LinearOrder (vector<int> order) {
	size = order.size();

	vector<int> thisOrder = this->order;
	for (int i = 0; i < size; i++) {
		thisOrder.push_back(order[i]);
	}
}

// Copy constructor
LinearOrder::LinearOrder (const LinearOrder& original) {
	size = original.size;

	for (int i = 0; i < size; i++) {
		order.push_back(original.order[i]);
	}
}

// Getter for order
vector<int> LinearOrder::getOrder () {
	vector<int> orderCopy;
	
	for (int node: order) {
		orderCopy.push_back(node);
	}

	return orderCopy;
}

// Print order
void LinearOrder::printOrder () {
	for (int node: order) {
		cout << node << " ";
	}
	cout << endl;
}

// Compute improvement for an insert move from nodeIndex to insertIndex
int LinearOrder::improvementForInsertMove(int** matrix, int nodeIndex, int insertIndex) {
	int improvement = 0;

	if (nodeIndex < insertIndex) {
		for (int i = insertIndex; i > nodeIndex; i--) {
			improvement += matrix[order[i]][order[nodeIndex]] - matrix[order[nodeIndex]][order[i]];
		}
	} else {
		for (int i = insertIndex; i < nodeIndex; i++) {
			improvement += matrix[order[nodeIndex]][order[i]] - matrix[order[i]][order[nodeIndex]];
		}
	}

	return improvement;
}

// Apply the insert move, the node at nodeIndex is moved to insertIndex
void LinearOrder::applyInsertMove (int nodeIndex, int insertIndex) {
	int node = order[nodeIndex];

	if (nodeIndex < insertIndex) {
		for (int i = nodeIndex; i < insertIndex; i++) {
			order[i] = order[i+1];
		}
	} else {
		for (int i = nodeIndex; i > insertIndex; i--) {
			order[i] = order[i-1];
		}
	}

	order[insertIndex] = node;
}

// Count inversions between two linear orders
int LinearOrder::countInversions (LinearOrder& other) {
	vector<int> originalOrder = this->getOrder();
	vector<int> otherOrder = other.getOrder();

	int *map = new int[size];
	for (int i = 0; i < size; i++) {
		map[originalOrder[i]] = i;
	}

	int *mapped = new int[size];
	for (int i = 0; i < size; i++) {
		mapped[i] = map[otherOrder[i]];
	}

	int inversions = 0;
	for (int i = 0; i < size; i++) {
		for (int j = i + 1; j < size; j++) {
			if (mapped[j] < mapped[i]) {
				inversions += 1;
			}
		}
	}

	return inversions;
}

// Implements crossover operator between two linear orders
LinearOrder LinearOrder::crossover (LinearOrder& other, float bias) {
	vector<int> originalOrder = this->getOrder();
	vector<int> otherOrder = other.getOrder();

	int *map = new int[size];
	for (int i = 0; i < size; i++) {
		map[originalOrder[i]] = i;
	}

	int *mapped = new int[size];
	for (int i = 0; i < size; i++) {
		mapped[i] = map[otherOrder[i]];
	}

	for (int i = 0; i < size; i++) {
		for (int j = 1; j < size; j++) {
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

	return LinearOrder(otherOrder);
}