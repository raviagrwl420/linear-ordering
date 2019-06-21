#ifndef MAX_FLOW_H
#define MAX_FLOW_H

#include <ranking.h>

#include <vector>
#include <deque>

#include <boost/config.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/read_dimacs.hpp>
#include <boost/graph/graph_utility.hpp>

using std::vector;
using std::deque;

using boost::adjacency_list_traits;
using boost::adjacency_list;
using boost::vecS;
using boost::directedS;
using boost::property;
using boost::vertex_name_t;
using boost::edge_capacity_t;
using boost::edge_residual_capacity_t;
using boost::edge_reverse_t;
using boost::property_map;
using boost::edge_capacity;
using boost::edge_reverse;
using boost::edge_residual_capacity;
using boost::graph_traits;
using boost::tie;
using boost::add_vertex;

typedef adjacency_list_traits<vecS, vecS, directedS> Traits;
typedef adjacency_list<
			vecS,
			vecS,
			directedS,
			property<vertex_name_t, std::string>,
			property<edge_capacity_t, long,
				property<edge_residual_capacity_t, long,
					property<edge_reverse_t, Traits::edge_descriptor> > >
> Graph;

int* solve_max_flow (Ranking& ranking, int** partial);

#endif