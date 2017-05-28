/**
 * \file
 * \author stvdedal@gmail.com
 *
 * \brief
 * Minimality check functionality
 */
#ifndef GSPAN_MINIMUM_CHECK_HPP
#define GSPAN_MINIMUM_CHECK_HPP

#include "gspan_types.hpp"
#include "gspan_helpers.hpp"

namespace gspan {

  /**
   * Insert into map MinExt new entry
   */
  template <typename MinExt, typename VI, typename MG, typename SBG,
    typename IGEdge>
  void
  add_min_edge(MinExt& min_ext,
               VI src,
               VI dst,
               const MG* prev_mg,
               IGEdge&& e,
               const SBG* prev_sbg)
  {
    /// first construct and insert key (Mined Graph or Ec)
    auto it =
      min_ext.insert(std::make_pair(make_ec(src,
                                            dst,
                                            prev_mg,
                                            e,
                                            *prev_sbg->input_graph(),
                                            boost::vertex_bundle_t(),
                                            boost::edge_bundle_t()),
                                    typename MinExt::mapped_type())).first;

    /// then construct and insert *subgraph*
    if (it == min_ext.begin()) {
      const MG& mg = it->first;
      it->second.emplace(it->second.begin(),
                         *edges(mg).first,
                         e,
                         &mg,
                         prev_sbg);
    }

    /// at last, remove all other (non-minimal entries)
    it = min_ext.begin();
    min_ext.erase(++it, min_ext.end());
  }

  /**
   * Insert into map MinExt new entry
   */
  template <typename MinExt, typename IG, typename IGEdge>
  void
  add_min_edge(MinExt& min_ext, IGEdge&& e, const IG* g)
  {
    using Traits = gspan_traits<IG>;

    /// first construct and insert key (Mined Graph or Ec)
    auto it =
      min_ext.insert(std::make_pair(make_ec(0,
                                            1,
                                            nullptr,
                                            e,
                                            *g,
                                            boost::vertex_bundle_t(),
                                            boost::edge_bundle_t()),
                                    typename MinExt::mapped_type())).first;

    /// then construct and insert *subgraph*
    if (it == min_ext.begin()) {
      const typename Traits::MG& mg = it->first;
      it->second.emplace(it->second.begin(), *edges(mg).first, e, &mg, g);
    }

    /// at last, remove all other (non-minimal entries)
    it = min_ext.begin();
    min_ext.erase(++it, min_ext.end());
  }

  /**
   * Collect backward edges into MinExt container
   * \param[out] min_ext    container
   * \param[in]  rmpath     sequence of edges that forms the right most path
   * \param[in]  mg         mined graph, in this context, it means minimal graph
   * \param[in]  sbgs       subgraphs of input graph
   * \param[in]  ig         input graph, in this context, it means tested graph
   */
  template <typename MinExt, typename RMPath, typename MG, typename SBG,
    typename IG>
  void
  enumerate_min_bck(MinExt& min_ext,
                    const RMPath& rmpath,
                    const MG& mg,
                    const std::list<SBG>& sbgs,
                    const IG& ig)
  {
    using MGV = typename MG::vertex_descriptor;
    using MGE = typename MG::edge_descriptor;
    using IGV = typename IG::vertex_descriptor;
    using IGE = typename IG::edge_descriptor;

    /**
     * iterate over rmpath vertices, beginning with first vertex
     * examine all backward edges: from right most vertex to rmpath vertex
     */
    MGV rmostv_mg = target(rmpath.front(), mg);
    for (MGE rme_mg : boost::adaptors::reverse(rmpath)) {
      if (!min_ext.empty())
        break;
      MGV rmv_mg = source(rme_mg, mg);
      const bool vl_less_eq = target_bundle(mg, rme_mg)
        <= v_bundle(mg, rmostv_mg);

      for (const SBG& s : sbgs) {
        IGV rmostv_ig = get_v_ig(s, rmostv_mg);
        IGV rmv_ig = get_v_ig(s, rmv_mg);
        IGE rme_ig = get_e_ig(s, rme_mg);
        // enumerate edges from rmostv_ig to rmv_ig
        for (IGE e : out_edges(rmostv_ig, ig)) {
          // skip already mapped edges
          if (get_e_mg(s, e) != MGE())
            continue;
          // only from from rmostv_ig to rmv_ig
          if (target(e, ig) != rmv_ig)
            continue;
          const auto& rme_val = e_bundle(ig, rme_ig);
          const auto& e_val = e_bundle(ig, e);
          if ((vl_less_eq && rme_val == e_val) || (rme_val < e_val)) {
            auto src = v_index(mg, rmostv_mg);
            auto dst = v_index(mg, rmv_mg);
            add_min_edge(min_ext, src, dst, &mg, e, &s);
            break;
          }
        }
      }
    }
  }

  /**
   * Collect forward edges into MinExt container
   * \param[out] min_ext    container
   * \param[in]  rmpath     sequence of edges that forms the right most path
   * \param[in]  mg         mined graph, in this context, it means minimal graph
   * \param[in]  sbgs       subgraphs of input graph
   * \param[in]  ig         input graph, in this context, it means tested graph
   */
  template <typename MinExt, typename RMPath, typename MG, typename SBG,
    typename IG>
  void
  enumerate_min_fwd(MinExt& min_ext,
                    const RMPath& rmpath,
                    const MG& mg,
                    const std::list<SBG>& sbgs,
                    const IG& ig)
  {
    using MGV = typename MG::vertex_descriptor;
    using MGE = typename MG::edge_descriptor;
    using IGV = typename IG::vertex_descriptor;
    using IGE = typename IG::edge_descriptor;

    const auto& vl_min = source_bundle(mg, rmpath.back());

    MGV rmostv_mg = target(rmpath.front(), mg);
    auto rmost_index = v_index(mg, rmostv_mg);

    /**
     * forward pure
     * examine forward edges: from right most vertex
     */
    for (const SBG& s : sbgs) {
      IGV u = get_v_ig(s, rmostv_mg);
      for (IGE e : out_edges(u, ig)) {
        IGV v = target(e, ig);
        // skip already mapped edges and vertices
        if (get_v_mg(s, v) != IGV())
          continue;
        if (vl_min > v_bundle(ig, v))
          continue;
        add_min_edge(min_ext, rmost_index, rmost_index + 1, &mg, e, &s);
      }
    }

    /**
     * forward rmpath
     * iterate over rmpath vertices, beginning with right most vertex
     * examine forward edges: from rmpath vertex
     */
    for (MGE rme_mg : rmpath) {
      if (!min_ext.empty())
        break;
      MGV rmv_mg = source(rme_mg, mg);
      auto rmv_index = v_index(mg, rmv_mg);

      for (const SBG& s : sbgs) {
        IGV u = get_v_ig(s, rmv_mg);
        for (IGE e : out_edges(u, ig)) {
          IGV v = target(e, ig);
          if (get_v_mg(s, v) != IGV())
            continue;
          if (vl_min > v_bundle(ig, v))
            continue;
          if ((target_bundle(mg, rme_mg) <= target_bundle(ig, e)
            && e_bundle(mg, rme_mg) == e_bundle(ig, e))
              || e_bundle(mg, rme_mg) < e_bundle(ig, e)) {
            add_min_edge(min_ext, rmv_index, rmost_index + 1, &mg, e, &s);
          }
        }
      }
    }
  }

  /**
   * Equality check of two edges
   * \param[in] e1, e2 edge to compare
   * \param[in] g1, g2 those graphs
   */
  template <typename EcGraph>
  bool
  is_equal(const typename EcGraph::edge_descriptor& e1,
           const EcGraph &g1,
           const typename EcGraph::edge_descriptor& e2,
           const EcGraph &g2)
  {
    if (source_index(g1, e1) != source_index(g2, e2))
      return false;
    if (target_index(g1, e1) != target_index(g2, e2))
      return false;
    if (source_bundle(g1, e1) != source_bundle(g2, e2))
      return false;
    if (target_bundle(g1, e1) != target_bundle(g2, e2))
      return false;
    if (e_bundle(g1, e1) != e_bundle(g2, e2))
      return false;
    return true;
  }

  /**
   * Perform minimality check
   * \param[in] tested_graph graph for testing
   */
  template <typename EcGraph>
  bool
  is_minimum(const EcGraph& tested_graph)
  {
    using Traits = gspan_traits<EcGraph>;

    const typename EcGraph::edges_size_type nedges = num_edges(tested_graph);
    typename EcGraph::edges_size_type n = 0;

    using MG = typename Traits::MG;
    using SBG = typename Traits::SBG;
    using MGE = typename MG::edge_descriptor;
    using IGE = typename EcGraph::edge_descriptor;

    /**
     * tested_dfsc is a representation of the tested_graph
     */
    std::vector<IGE> tested_dfsc(nedges);

    /**
     * the sequence of MinExt is a newly constructed tested_graph (MG)
     * with mappings (std::list<SBG>) to original tested_graph.
     * MinExt is a map, but its size is 1, it contains only one *minimal* MG
     */
    using MinExt = std::map<MG, std::list<SBG>, gspan::edgecode_compare_dfs>;
    std::vector<MinExt> min_exts;
    min_exts.reserve(nedges);

    min_exts.push_back(MinExt());
    n = nedges - 1;
    for (IGE e : edges(tested_graph)) {
      add_min_edge(min_exts.back(), e, &tested_graph);
      tested_dfsc[n--] = e;
    }

    for (n = 0; n < nedges; ++n) {

      const typename MinExt::value_type& last = *min_exts[n].begin();
      MGE last_edge = *edges(last.first).first;

      if (!is_equal(last_edge, last.first, tested_dfsc[n], tested_graph)) {
        return false;
      }

      std::vector<MGE> rmpath;
      rmpath.reserve(num_edges(last.first));
      for (MGE e : rmpath_edges(last.first))
        rmpath.push_back(e);

      min_exts.push_back(MinExt());

      enumerate_min_bck(min_exts.back(),
                        rmpath,
                        last.first,
                        last.second,
                        tested_graph);
      if (!min_exts.back().empty()) {
        continue;
      }

      enumerate_min_fwd(min_exts.back(),
                        rmpath,
                        last.first,
                        last.second,
                        tested_graph);
      if (!min_exts.back().empty()) {
        continue;
      }

      break;
    }

    return true;
  }

} // namespace gspan

#endif
