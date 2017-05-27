#include "gspan_subgraph_lists.hpp"
#include "gspan_subgraph_tree.hpp"
#include "gspan_edgecode_compare.hpp"
#include "gspan_edgecode_tree.hpp"
#include <iostream>

#include <boost/graph/adjacency_list.hpp>

using namespace gspan;

template <typename V, typename ViMap, typename VVMap>
std::ostream&
print_vertex(std::ostream& s, V v, ViMap vimap, VVMap vvmap)
{
  s << get(vimap, v) << " (" << get(vvmap, v) << ")";
  return s;
}

template <typename G>
void
print_out_edges(typename boost::graph_traits<G>::vertex_descriptor v,
                const G& g)
{
  typename boost::property_map<G, boost::vertex_index_t>::const_type vindex =
    get(boost::vertex_index_t(), g);
  typename boost::property_map<G, boost::edge_index_t>::const_type eindex =
    get(boost::edge_index_t(), g);

  typename boost::property_map<G, boost::vertex_bundle_t>::const_type vbundle =
    get(boost::vertex_bundle_t(), g);
  typename boost::property_map<G, boost::edge_bundle_t>::const_type ebundle =
    get(boost::edge_bundle_t(), g);

  std::cout << "out_edges of vertex: " << get(vindex, v) << "\tout_degree: "
  << out_degree(v, g) << std::endl;
  for (auto p = out_edges(v, g); p.first != p.second; ++p.first) {
    std::cout << " edge : " << get(eindex, *p.first);
    std::cout << "\tsource: ";
    print_vertex(std::cout, source(*p.first, g), vindex, vbundle);
    std::cout << "\ttarget: ";
    print_vertex(std::cout, target(*p.first, g), vindex, vbundle);
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

int
main()
{
  typedef edgecodetree<char, char, boost::undirected_tag> Ec;

  enum V
  {
    V0, V1, V2, V3, V4, V5, N
  };
  char vname[N] = { 'A', 'B', 'C', 'D', 'E', 'F' };

  Ec ec0(V0, V1, vname[V0], vname[V1], 'a');
  Ec ec1(V1, V2, vname[V1], vname[V2], 'a', &ec0);
  Ec ec2(V1, V3, vname[V1], vname[V3], 'a', &ec1);
  Ec ec3(V0, V4, vname[V0], vname[V4], 'a', &ec2);
  Ec ec4(V1, V5, vname[V1], vname[V5], 'a', &ec3);
  Ec ec5(V0, V5, vname[V0], vname[V5], 'a', &ec4);
  Ec ec6(V2, V5, vname[V2], vname[V5], 'a', &ec5);

  print_dfsc(ec6);

  gspan::edgecode_compare_dfs cmp_dfs;
  cmp_dfs(ec6, ec6);

  auto me0 = *ec0.edges().first;

  typedef boost::property<boost::vertex_name_t, std::string> VP;
  typedef boost::property<boost::edge_index_t,   unsigned int> EP;
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VP, EP> InputGraph;
  InputGraph ig(2);

  auto ie = add_edge(0, 1, ig).first;

#if 0
  subgraph_tree<InputGraph, Ec> sbg(me0, ie, &ec6, &ig);
  sbg.m2i_vert_map();
  sbg.m2i_edge_map();
  sbg.i2m_vert_map();
  sbg.i2m_edge_map();

#endif
  subgraph_lists<subgraph_tree<InputGraph, Ec>> li;
  li.insert(me0, ie, &ec6, &ig);

  std::cerr << std::endl << "Done" << std::endl;
}
