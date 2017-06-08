/**
 * \file
 * \author stvdedal@gmail.com
 *
 * \brief
 * gSpan algorithm
 */
#ifndef GSPAN_HPP
#define GSPAN_HPP

#include "gspan_types.hpp"
#include "gspan_helpers.hpp"
#include "gspan_minimum_check.hpp"

/// gspan algorithm
namespace gspan {

struct one_graph_tag {

};
struct many_graphs_tag {
};

template <typename SG>
unsigned int
support(const SG& sg, one_graph_tag)
{
    return sg.begin()->second.aut_list_size;
}

template <typename SG>
unsigned int
support(const SG& sg, many_graphs_tag)
{
    return sg.size();
}

template <typename Ext, typename VI, typename MG, typename IGEdge,
          typename SBG, typename VPT, typename EPT>
void
add_edge(Ext& ext,
         VI src,
         VI dst,
         const MG* prev_mg,
         IGEdge&& e,
         const SBG* prev_sbg,
         VPT vpt,
         EPT ept)
{
    const typename SBG::InputGraph* ig = prev_sbg->input_graph();
    auto it = ext.insert(std::make_pair(make_ec(src,
                                        dst,
                                        prev_mg,
                                        e,
                                        *ig,
                                        vpt,
                                        ept),
                                        typename Ext::mapped_type())).first;

    const MG& mg = it->first;
    it->second[ig].insert(*edges(mg).first, e, &mg, prev_sbg);
}

template <typename Ext, typename IGEdge, typename IG, typename VPT,
          typename EPT>
void
add_edge(Ext& ext, IGEdge&& e, const IG* ig, VPT vpt, EPT ept)
{
    auto it = ext.insert(std::make_pair(make_ec(0,
                                        1,
                                        nullptr,
                                        e,
                                        *ig,
                                        vpt,
                                        ept),
                                        typename Ext::mapped_type())).first;
    const typename Ext::key_type& mg = it->first;
    it->second[ig].insert(*edges(mg).first, e, &mg, ig);
}

template <typename RExt, typename MG, typename IG, typename SBGS,
          typename VPT, typename EPT>
void
enumerate(RExt& r_ext,
          const MG& mg,
          const IG& ig,
          const SBGS& sbgs,
          VPT vpt,
          EPT ept)
{
    /**
     * R edges will be
     * IF
     * 1) forward edge  : src is rmpath vertex AND dst is new vertex OR
     * 2) backward edge : src is rmost vertex AND dst is any rmpath vertex
     * ELSE
     * X edges
     */

    using MGV = typename boost::graph_traits<MG>::vertex_descriptor;
    using MGE = typename boost::graph_traits<MG>::edge_descriptor;
    using IGV = typename boost::graph_traits<IG>::vertex_descriptor;
    using IGE = typename boost::graph_traits<IG>::edge_descriptor;

    // Right most path edges
    // is a map
    // Key   : vertex index
    // Value : rmpath edge
    std::vector<MGE> vsrc_edges(num_edges(mg));

    // Right most path vertex mask
    // size == num_vertices(mg)
    // true: vertex on rm path; false otherwise
    std::vector<bool> rmpath_vertex_mask(num_edges(mg), false);

    for (MGE e : rmpath_edges(mg)) {
        MGV v = source(e, mg);
        vsrc_edges[v_index(mg, v)] = e;
        rmpath_vertex_mask[v_index(mg, v)] = true;
    }
    MGV rmost_mg = target(*rmpath_edges(mg).first, mg);
    rmpath_vertex_mask[v_index(mg, rmost_mg)] = true;

    const auto& vl_min = v_bundle(mg, 0);

    for (const auto& s : sbgs.all_list) {

        // from right most vertex
        IGV rmost_ig = get_v_ig(s, rmost_mg);
        for (IGE e_ig : out_edges(rmost_ig, ig)) {
            IGV v = target(e_ig, ig);

            // skip edges in MinedGraph
            if (get_e_mg(s, e_ig) != MGE())
                continue;

            MGV v_mg = get_v_mg(s, v);
            if (v_mg == MGV()) {
                // R forward

                // Partial pruning
                if (get(vpt, ig, v) >= vl_min) {
                    auto src = v_index(mg, rmost_mg);
                    auto dst = v_index(mg, rmost_mg) + 1;
                    add_edge(r_ext, src, dst, &mg, e_ig, &s, vpt, ept);
                }
            }
            else if (rmpath_vertex_mask[v_index(mg, v_mg)]) {
                // R backward

                // Partial pruning
                MGE rmpath_e_mg = vsrc_edges[v_index(mg, v_mg)];
                IGE rmpath_e_ig = get_e_ig(s, rmpath_e_mg);
                BOOST_ASSERT(rmpath_e_mg != MGE());
                BOOST_ASSERT(get(ept, ig, rmpath_e_ig) == e_bundle(mg, rmpath_e_mg));
                BOOST_ASSERT(get(vpt, ig, source(rmpath_e_ig, ig)) == source_bundle(mg,
                             rmpath_e_mg));
                BOOST_ASSERT(get(vpt, ig, target(rmpath_e_ig, ig)) == target_bundle(mg,
                             rmpath_e_mg));

                if (get(ept, ig, e_ig) > get(ept, ig, rmpath_e_ig) ||
                        (get(ept, ig, e_ig) == get(ept, ig, rmpath_e_ig) &&
                         get(vpt, ig, rmost_ig) >= get(vpt, ig, target(rmpath_e_ig, ig)) )) {

                    auto src = v_index(mg, rmost_mg);
                    auto dst = v_index(mg, v_mg);
                    add_edge(r_ext, src, dst, &mg, e_ig, &s, vpt, ept);
                }
            }

        } // for out_edges(rmost_ig)


        for (MGE rmpath_e_mg : rmpath_edges(mg)) {
            IGE rmpath_e_ig = get_e_ig(s, rmpath_e_mg);
            IGV rmpath_v_ig = source(rmpath_e_ig, ig);

            for (IGE e_ig : out_edges(rmpath_v_ig, ig)) {
                IGV u = target(e_ig, ig);
                // skip edges and vertices in MinedGraph
                if (get_e_mg(s, e_ig) != MGE() || get_v_mg(s, u) != MGV())
                    continue;

                if (get(ept, ig, rmpath_e_ig) < get(ept, ig, e_ig) ||
                        (get(ept, ig, rmpath_e_ig) == get(ept, ig, e_ig) &&
                         get(vpt, ig, target(rmpath_e_ig, ig)) <= get(vpt, ig, u) ) ) {

                    // R forward
                    auto src = v_index(mg, source(rmpath_e_mg, mg));
                    auto dst = v_index(mg, rmost_mg) + 1;
                    add_edge(r_ext, src, dst, &mg, e_ig, &s, vpt, ept);
                }
            }
        }
    }
}

template <typename RExt,
          typename IG,
          typename VPT, typename EPT>
void
enumerate_one_edges(RExt& r_ext, const IG* ig, VPT vpt, EPT ept)
{
    for (auto v : vertices(*ig))
        for (auto e : out_edges(v, *ig))
            add_edge(r_ext, e, ig, vpt, ept);
}

template <typename IG, typename Result, typename SupCalcType, typename VPTag,
          typename EPTag>
class Alg {
public:
    using Traits = gspan_traits<IG, VPTag, EPTag>;
    using InputGraph = typename Traits::IG;
    using MinedGraph = typename Traits::MG;
    using SG = typename Traits::SG;
    using RExt = typename Traits::RExt;
    using XExt = typename Traits::XExt;

    Alg(Result result, unsigned int minsup, VPTag vptag, EPTag eptag)
        : vptag_(vptag), eptag_(eptag), minsup_(minsup), result_(result),
          subgraph_mining_count_(0)
    {
    }

    void
    run(const RExt& r_ext);

    void
    subgraph_mining(const MinedGraph& mg, const SG& sg, unsigned int supp);

    VPTag vptag_;
    EPTag eptag_;
    unsigned int minsup_;
    Result result_;

    std::size_t subgraph_mining_count_;
};

template <typename IG,
          typename Result,
          typename SupCalcType,
          typename VPTag,
          typename EPTag>
void
Alg<IG, Result, SupCalcType, VPTag, EPTag>::run(const RExt& r_ext)
{
    for (const auto& ext : r_ext) {
        unsigned int supp = support(ext.second, SupCalcType());
        if (minsup_ <= supp) {
            subgraph_mining(ext.first, ext.second, supp);
        }
    }
}

template <typename IG, typename Result, typename SupCalcType, typename VPTag,
          typename EPTag>
void
Alg<IG, Result, SupCalcType, VPTag, EPTag>::subgraph_mining(
    const MinedGraph& mg,
    const SG& sg,
    unsigned int supp)
{
    ++subgraph_mining_count_;

    if (!is_minimum(mg)) {
        return;
    }

    result_(mg, sg, supp);

    RExt r_edges;
    for (const auto& x : sg) {
        enumerate(r_edges, mg, *x.first, x.second, vptag_, eptag_);
    }

    for (const auto& ext : r_edges) {
        unsigned int supp2 = support(ext.second, SupCalcType());
        if (minsup_ <= supp2) {
            subgraph_mining(ext.first, ext.second, supp2);
        }
    }
}

} // namespace gspan

/**
 * Perform gSpan for one graph
 */
template <typename IG, typename Result, typename VPTag, typename EPTag>
void
gspan_one_graph(const IG& ig,
                unsigned int minsup,
                Result result,
                VPTag vptag,
                EPTag eptag)
{
    typename gspan_traits<IG, VPTag, EPTag>::RExt r_ext;
    gspan::enumerate_one_edges(r_ext, &ig, vptag, eptag);

    using Alg = gspan::Alg<IG, Result, gspan::one_graph_tag, VPTag, EPTag>;
    Alg alg(result, minsup, vptag, eptag);
    alg.run(r_ext);
}

/**
 * Perform gSpan for many graphs
 */
template <typename IGIter, typename Result, typename VPTag, typename EPTag>
void
gspan_many_graphs(const IGIter ig_begin,
                  const IGIter ig_end,
                  unsigned int minsup,
                  Result result,
                  VPTag vptag,
                  EPTag eptag)
{
    using IG = typename std::iterator_traits<IGIter>::value_type;
    using Alg = gspan::Alg<IG, Result, gspan::many_graphs_tag, VPTag, EPTag>;
    Alg alg(result, minsup, vptag, eptag);

    typename gspan_traits<IG, VPTag, EPTag>::RExt r_ext;
    for (IGIter g = ig_begin; g != ig_end; ++g) {
        gspan::enumerate_one_edges(r_ext, &*g, vptag, eptag);
    }

    alg.run(r_ext);

    std::cerr << "subgraph_mining_count=" << alg.subgraph_mining_count_ <<
              std::endl;
}

#endif
