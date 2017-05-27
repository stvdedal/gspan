#ifndef GSPAN_SUBGRAPH_TREE_H_
#define GSPAN_SUBGRAPH_TREE_H_

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>

#include <boost/assert.hpp>

#include <vector>
#include <limits>

namespace gspan {

  template <typename IG, typename MG>
  class subgraph_tree
  {
  public:
    typedef IG InputGraph;
    typedef MG MinedGraph;
    typedef boost::graph_traits<InputGraph> IGT;
    typedef boost::graph_traits<MinedGraph> MGT;

    subgraph_tree(const typename MGT::edge_descriptor& mined_edge,
                  const typename IGT::edge_descriptor& input_edge,
                  const MinedGraph* mined_graph,
                  const InputGraph* input_graph);

    subgraph_tree(const typename MGT::edge_descriptor& mined_edge,
                  const typename IGT::edge_descriptor& input_edge,
                  const MinedGraph* mined_graph,
                  const subgraph_tree* prev);

    const InputGraph* input_graph() const
    {
      return _ig;
    }

    //
    // map Mined graph vertex to Input graph vertex
    //
    using InputGraphVerts = std::vector<typename IGT::vertex_descriptor>;
    using InputGraphVertsIter = typename InputGraphVerts::const_iterator;
    using MinedGraphVertIdMap = typename boost::property_map<MinedGraph, boost::vertex_index_t>::const_type;
    using Mined2InputVertMap = boost::iterator_property_map<InputGraphVertsIter, MinedGraphVertIdMap>;

    Mined2InputVertMap
    m2i_vert_map() const;

    //
    // map Mined graph edge to Input graph edge
    //
    using InputGraphEdges = std::vector<typename IGT::edge_descriptor>;
    using InputGraphEdgesIter = typename InputGraphEdges::const_iterator;
    using MinedGraphEdgeIdMap = typename boost::property_map<MinedGraph, boost::edge_index_t>::const_type;
    using Mined2InputEdgeMap = boost::iterator_property_map<InputGraphEdgesIter, MinedGraphEdgeIdMap>;

    Mined2InputEdgeMap
    m2i_edge_map() const;

    //
    // map Input graph vertex to Mined graph vertex
    //
    using MinedGraphVerts = std::vector<typename MGT::vertex_descriptor>;
    using MinedGraphVertsIter = typename MinedGraphVerts::const_iterator;
    using InputGraphVertIdMap = typename boost::property_map<InputGraph, boost::vertex_index_t>::const_type;
    using Input2MinedVertMap = boost::iterator_property_map<MinedGraphVertsIter, InputGraphVertIdMap>;

    Input2MinedVertMap
    i2m_vert_map() const;

    //
    // map Input graph edge to Mined graph edge
    //
    using MinedGraphEdges = std::vector<typename MGT::edge_descriptor>;
    using MinedGraphEdgesIter = typename MinedGraphEdges::const_iterator;
    using InputGraphEdgeIdMap = typename boost::property_map<InputGraph, boost::edge_index_t>::const_type;
    using Input2MinedEdgeMap = boost::iterator_property_map<MinedGraphEdgesIter, InputGraphEdgeIdMap>;

    Input2MinedEdgeMap
    i2m_edge_map() const;

    // subgraphs is automorphic if they belong to the same graph
    // and contains the same set of edges
    static bool
    is_automorphic(const subgraph_tree& lhs, const subgraph_tree& rhs);

  private:
    const subgraph_tree* _prev;

    const MinedGraph* _mg;
    const InputGraph* _ig;

    typename MGT::edge_descriptor _mg_edge;
    typename IGT::edge_descriptor _ig_edge;

    // size  == num_vertices(MinedGraph)
    // indexed by MinedGraph vertex_index
    // values are InputGraph vertex_descriptor
    InputGraphVerts _ig_vertices;

    // size == num_edges(MinedGraph)
    // indexed by MinedGraph edge_index
    // values are InputGraph edge_descriptor
    InputGraphEdges _ig_edges;

    // size == num_vertices(InputGraph)
    // indexed by InputGraph vertex_index
    // values are MinedGraph vertex_descriptor
    MinedGraphVerts _mg_vertices;

    // size == num_edges(InputGraph)
    // indexed by InputGraph edge_index
    // values are MinedGraph edge_descriptor
    MinedGraphEdges _mg_edges;

  };

  //
  // the first edge
  //
  template <typename IG, typename MG>
  subgraph_tree<IG, MG>::subgraph_tree(const typename MGT::edge_descriptor& mined_edge,
                                       const typename IGT::edge_descriptor& input_edge,
                                       const MinedGraph* mined_graph,
                                       const InputGraph* input_graph)
    : _prev(nullptr), _mg(mined_graph), _ig(input_graph), _mg_edge(mined_edge),
      _ig_edge(input_edge), _ig_vertices(), _ig_edges(), _mg_vertices(),
      _mg_edges()
  {
    MinedGraphVertIdMap mvi = get(boost::vertex_index_t(), *_mg);
    InputGraphVertIdMap ivi = get(boost::vertex_index_t(), *_ig);
    InputGraphEdgeIdMap iei = get(boost::edge_index_t(), *_ig);

    // map Mined graph vertex to Input graph vertex
    _ig_vertices.resize(2);
    _ig_vertices[get(mvi, source(_mg_edge, *_mg))] = source(_ig_edge, *_ig);
    _ig_vertices[get(mvi, target(_mg_edge, *_mg))] = target(_ig_edge, *_ig);

    // map Mined graph edge to Input graph edge
    _ig_edges.push_back(_ig_edge);

    // map Input graph vertex to Mined graph vertex
    _mg_vertices.resize(num_vertices(*_ig), InputGraph::null_vertex());
    _mg_vertices[get(ivi, source(_ig_edge, *_ig))] = source(_mg_edge, *_mg);
    _mg_vertices[get(ivi, target(_ig_edge, *_ig))] = target(_mg_edge, *_mg);

    // map Input graph edge to Mined graph edge
    _mg_edges.resize(num_edges(*_ig)); // value is default constructed MinedGraph::edge_descriptor
    _mg_edges[get(iei, _ig_edge)] = _mg_edge;
  }

  //
  // the all other edges
  //
  template <typename IG, typename MG>
  subgraph_tree<IG, MG>::subgraph_tree(const typename MGT::edge_descriptor& mined_edge,
                                       const typename IGT::edge_descriptor& input_edge,
                                       const MinedGraph* mined_graph,
                                       const subgraph_tree* prev)
    : _prev(prev), _mg(mined_graph), _ig(prev->_ig), _mg_edge(mined_edge),
      _ig_edge(input_edge), _ig_vertices(prev->_ig_vertices),
      _ig_edges(prev->_ig_edges), _mg_vertices(prev->_mg_vertices),
      _mg_edges(prev->_mg_edges)
  {
    MinedGraphVertIdMap mvi = get(boost::vertex_index_t(), *_mg);
    InputGraphVertIdMap ivi = get(boost::vertex_index_t(), *_ig);
    InputGraphEdgeIdMap iei = get(boost::edge_index_t(), *_ig);

    BOOST_ASSERT(get(mvi, target(_mg_edge, *_mg)) <= _ig_vertices.size());

    // map Mined graph vertex to Input graph vertex
    if (get(mvi, target(_mg_edge, *_mg)) == _ig_vertices.size()) {
      _ig_vertices.resize(_ig_vertices.size() + 1);
      _ig_vertices[get(mvi, target(_mg_edge, *_mg))] = target(_ig_edge, *_ig);
    }

    // map Mined graph edge to Input graph edge
    _ig_edges.push_back(_ig_edge);

    // map Input graph vertex to Mined graph vertex
    _mg_vertices[get(ivi, source(_ig_edge, *_ig))] = source(_mg_edge, *_mg);
    _mg_vertices[get(ivi, target(_ig_edge, *_ig))] = target(_mg_edge, *_mg);

    // map Input graph edge to Mined graph edge
    _mg_edges[get(iei, _ig_edge)] = _mg_edge;
  }

  template <typename IG, typename MG>
  typename subgraph_tree<IG, MG>::Mined2InputVertMap
  subgraph_tree<IG, MG>::m2i_vert_map() const
  {
    return Mined2InputVertMap(_ig_vertices.begin(),
                              get(boost::vertex_index_t(), *_mg));
  }

  template <typename IG, typename MG>
  typename subgraph_tree<IG, MG>::Mined2InputEdgeMap
  subgraph_tree<IG, MG>::m2i_edge_map() const
  {
    return Mined2InputEdgeMap(_ig_edges.begin(),
                              get(boost::edge_index_t(), *_mg));
  }

  template <typename IG, typename MG>
  typename subgraph_tree<IG, MG>::Input2MinedVertMap
  subgraph_tree<IG, MG>::i2m_vert_map() const
  {
    return Input2MinedVertMap(_mg_vertices.begin(),
                              get(boost::vertex_index_t(), *_ig));
  }

  template <typename IG, typename MG>
  typename subgraph_tree<IG, MG>::Input2MinedEdgeMap
  subgraph_tree<IG, MG>::i2m_edge_map() const
  {
    return Input2MinedEdgeMap(_mg_edges.begin(),
                              get(boost::edge_index_t(), *_ig));
  }

  template <typename IG, typename MG>
  bool
  subgraph_tree<IG, MG>::is_automorphic(const subgraph_tree& lhs,
                                        const subgraph_tree& rhs)
  {
    const std::size_t nedges = lhs._mg_edges.size();
    static const typename MGT::edge_descriptor null_edge;

    if (lhs._ig != rhs._ig)
      return false;

    BOOST_ASSERT(nedges == rhs._mg_edges.size());

    for (std::size_t i = 0; i < nedges; ++i) {
      bool lhs_edge_mapped = lhs._mg_edges[i] != null_edge;
      bool rhs_edge_mapped = rhs._mg_edges[i] != null_edge;
      if (lhs_edge_mapped != rhs_edge_mapped)
        return false;
    }

    return true;
  }

  template <typename IG, typename MG>
  bool is_automorphic(const subgraph_tree<IG, MG>& lhs, const subgraph_tree<IG, MG>& rhs)
  {
    return subgraph_tree<IG, MG>::is_automorphic(lhs, rhs);
  }

  template <typename IG, typename MG>
  bool is_automorphic(const subgraph_tree<IG, MG>* lhs, const subgraph_tree<IG, MG>* rhs)
  {
    return is_automorphic(*lhs, *rhs);
  }

  template<typename SBG, typename MGV>
  auto get_v_ig(const SBG& s, MGV&& v_mg)
  {
    return get(s.m2i_vert_map(), v_mg);
  }

  template<typename SBG, typename MGE>
  auto get_e_ig(const SBG& s, MGE&& e_mg)
  {
    return get(s.m2i_edge_map(), e_mg);
  }

  template<typename SBG, typename IGV>
  auto get_v_mg(const SBG& s, IGV&& v_ig)
  {
    return get(s.i2m_vert_map(), v_ig);
  }

  template<typename SBG, typename IGE>
  auto get_e_mg(const SBG& s, IGE&& e_ig)
  {
    return get(s.i2m_edge_map(), e_ig);
  }

} // end namespace gspan

#endif
