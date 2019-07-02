#include<max_flow.h>

vector<int> solve_max_flow (Ranking& ranking, int** partial) {
	int size = ranking.getSize();
	int** matrix = ranking.getMatrix();
	int* weights = ranking.getPotentials();

	Graph graph;
	long flow;

	property_map<Graph, edge_capacity_t>::type capacity = get(edge_capacity, graph);
	property_map<Graph, edge_reverse_t>::type reverse_edges = get(edge_reverse, graph);
	property_map<Graph, edge_residual_capacity_t>::type residual_capacity = get(edge_residual_capacity, graph);

	std::vector<Traits::vertex_descriptor> verts;
	for (int i = 0; i < size; i++) {
		verts.push_back(add_vertex(graph));
	}
	verts.push_back(add_vertex(graph)); // Add source vertex
	verts.push_back(add_vertex(graph)); // Add sink vertex
	verts.push_back(add_vertex(graph)); // Add dummy vertex

	Traits::vertex_descriptor s, t, d;
	s = verts[size];
	t = verts[size + 1];
	d = verts[size + 2];

	Traits::edge_descriptor e1, e2;
	bool in1, in2;

	// For vertices with positive weights: add edge from source to vertex
	// For vertices with negative weights: add edge from vertex to sink
	for (int i = 0; i < size; i++) {
		if (weights[i] >= 0) {
			tie(e1, in1) = add_edge(s, verts[i], graph);
			tie(e2, in2) = add_edge(verts[i], s, graph);
			capacity[e1] = weights[i];
			capacity[e2] = 0;
			reverse_edges[e1] = e2;
			reverse_edges[e2] = e1;
		} else {
			tie(e1, in1) = add_edge(verts[i], t, graph);
			tie(e2, in2) = add_edge(t, verts[i], graph);
			capacity[e1] = -weights[i];
			capacity[e2] = 0;
			reverse_edges[e1] = e2;
			reverse_edges[e2] = e1;
		}
	}

	// Add edges with MAX capacity in the reverse direction of fixed edges
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (partial[i][j] == 1) {
				tie(e1, in1) = add_edge(verts[i], verts[j], graph);
				tie(e2, in2) = add_edge(verts[j], verts[i], graph);
				capacity[e2] = INT_MAX; // Capacity MAX in reverse direction
				capacity[e1] = 0; // Capacity Zero in forward direction
				reverse_edges[e1] = e2;
				reverse_edges[e2] = e1;
			}
		}
	}

	// Run the max flow algorithm
	flow = push_relabel_max_flow(graph, s, t);

	// Initialize an all zero array to store the solution
	vector<int> partition(size, 0);

	// Construct the residual graph
	Graph residualGraph;
	for (int i = 0; i < size + 3; i++) {
		add_vertex(residualGraph);
	}

	graph_traits<Graph>::edge_iterator ei, e_end;
	for (tie(ei, e_end) = edges(graph); ei != e_end; ++ei) {
		if (residual_capacity[*ei] > 0) {
			Traits::vertex_descriptor u = source(*ei, graph);
			Traits::vertex_descriptor v = target(*ei, graph);
			add_edge(u, v, residualGraph);
		}
	}

	graph_traits<Graph>::out_edge_iterator oei, oe_end;
	deque<Traits::vertex_descriptor> solution_vertices;
	solution_vertices.push_back(s);
	while(!solution_vertices.empty()) {
		Traits::vertex_descriptor first = solution_vertices.front();
		solution_vertices.pop_front();

		for (tie(oei, oe_end) = out_edges(first, residualGraph); oei != oe_end; ++oei) {
			Traits::vertex_descriptor v = target(*oei, residualGraph);
			if (partition[v] != 1) {
				partition[v] = 1;
				solution_vertices.push_back(v);
			}
		}	
	}

	verts.clear();
	solution_vertices.clear();

	return partition;
}