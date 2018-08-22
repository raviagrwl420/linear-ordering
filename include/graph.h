#ifndef GRAPH_H
#define GRAPH_H

#include<ranking.h>
#include<io.h>

#include<boost/functional/hash.hpp>
#include<boost/graph/adjacency_matrix.hpp>
#include<boost/graph/strong_components.hpp>

using namespace boost;

typedef adjacency_matrix<directedS> MatrixGraph;
typedef graph_traits<adjacency_matrix<directedS>>::vertex_descriptor Vertex;

int strongly_connected_components (Ranking&);

#endif