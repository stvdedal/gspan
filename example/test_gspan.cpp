#include "gspan.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <iostream>

using namespace boost;

typedef property<vertex_name_t, char> VP;
typedef property<edge_index_t, unsigned int, property<edge_name_t, char> > EP;
typedef adjacency_list<vecS, vecS, undirectedS, VP, EP> InputGraph;
typedef gspan_traits<InputGraph, vertex_name_t, edge_name_t> GspanTraits;

void
result(const GspanTraits::MG& mg, const GspanTraits::SG& sg, int support)
{
  std::cout << "=================== result ===================" << std::endl;
  std::cout << "mined graph with support " << support << ":" << std::endl;
  print_dfsc(mg, std::cout);

  std::cout << std::endl;
}

int
main()
{
  InputGraph ig;

  auto v0 = add_vertex('A', ig);
  auto v1 = add_vertex('B', ig);
  auto v2 = add_vertex('C', ig);
  auto v3 = add_vertex('D', ig);
  auto v4 = add_vertex('E', ig);

  auto e0 = add_edge(v0, v1, 0, ig).first;
  auto e1 = add_edge(v1, v2, 1, ig).first;
  auto e2 = add_edge(v2, v3, 2, ig).first;
  auto e3 = add_edge(v3, v4, 3, ig).first;

  put(edge_name_t(), ig, e0, '_');
  put(edge_name_t(), ig, e1, '_');
  put(edge_name_t(), ig, e2, '_');
  put(edge_name_t(), ig, e3, '_');

  gspan_one_graph(ig, 1, result, vertex_name_t(), edge_name_t());
}
