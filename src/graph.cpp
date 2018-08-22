#include<graph.h>

int strongly_connected_components (Ranking& ranking) {
	ranking.initiateMatrixFromFile();

	int size = ranking.getSize();
	int **matrix = ranking.getMatrix();

	MatrixGraph mg(size);

	for (int i = 0; i < size; i++) {
		for (int j = i; j < size; j++) {
			if (matrix[i][j] > matrix[j][i]) {
				add_edge(i, j, mg);
			} else if (matrix[i][j] < matrix[j][i]) {
				add_edge(j, i, mg);
			} else {
				if (matrix[i][j] == 0 && matrix[j][i] == 0) {
					// Do nothing
				} else {
					add_edge(i, j, mg);
					add_edge(j, i, mg);
				}
			}
		}
	}

	vector<int> component(num_vertices(mg)), discover_time(num_vertices(mg));
	vector<default_color_type> color(num_vertices(mg));
	vector<Vertex> root(num_vertices(mg));

	int num = strong_components(mg, make_iterator_property_map(component.begin(), get(vertex_index, mg)), 
                              root_map(make_iterator_property_map(root.begin(), get(vertex_index, mg))).
                              color_map(make_iterator_property_map(color.begin(), get(vertex_index, mg))).
                              discover_time_map(make_iterator_property_map(discover_time.begin(), get(vertex_index, mg))));

	return num;
}