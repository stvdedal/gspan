/**
 * \file
 * \author stvdedal@gmail.com
 *
 * \brief
 * Type declarations
 */
#ifndef GSPAN_TYPES_HPP
#define GSPAN_TYPES_HPP

#include "gspan_edgecode_tree.hpp"
#include "gspan_edgecode_compare.hpp"
#include "gspan_subgraph_tree.hpp"
#include "gspan_subgraph_lists.hpp"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>

#include <map>
#include <utility>

template <typename IG_, typename VPTag = boost::vertex_bundle_t,
  typename EPTag = boost::edge_bundle_t>
class gspan_traits
{
  using vp_ig_pmap = typename boost::property_map<IG_, VPTag>::const_type;
  using ep_ig_pmap = typename boost::property_map<IG_, EPTag>::const_type;
  using vi_ig_pmap = typename boost::property_map<IG_, boost::vertex_index_t>::const_type;
  using ei_ig_pmap = typename boost::property_map<IG_, boost::edge_index_t>::const_type;
public:

  /// type of the Input graph
  using IG = IG_;

  /// properties
  using VI = typename boost::property_traits<vi_ig_pmap>::value_type;
  using EI = typename boost::property_traits<ei_ig_pmap>::value_type;
  using VP = typename boost::property_traits<vp_ig_pmap>::value_type;
  using EP = typename boost::property_traits<ep_ig_pmap>::value_type;

  using directed_category = typename boost::graph_traits<IG>::directed_category;

  /// type of the Mined graph
  using MG = gspan::edgecodetree< VP, EP, directed_category, VI, EI>;

  /// subgraph (SBG)
  using SBG = gspan::subgraph_tree<IG, MG>;

  /// subgraph (SBG) lists
  using SBGS = gspan::subgraph_lists<SBG>;

  /// Edge extentions
  using SG = std::map<const IG*, SBGS>;
  using RExt = std::map<MG, SG, gspan::edgecode_compare_dfs>;
  using XExt = std::map<MG, SG, gspan::edgecode_compare_lex>;
  //using MinExt = std::map<MG, std::list<SBG>, gspan::edgecode_compare_dfs>;
};

#endif
