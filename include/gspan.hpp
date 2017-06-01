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

    MGV rmost_v = target(*rmpath_edges(mg).first, mg);
    std::vector<bool> rmpath_vi_mask(num_edges(mg), false);
    for (MGE e : rmpath_edges(mg))
        rmpath_vi_mask[source_index(mg, e)] = true;
    rmpath_vi_mask[v_index(mg, rmost_v)] = true;

    for (const auto& s : sbgs.all_list) {

        // for each vertices in MinedGraph
        for (MGV vmg : vertices(mg)) {

            bool on_rmpath = rmpath_vi_mask[v_index(mg, vmg)];
            bool is_rmost = vmg == rmost_v;

            if (!on_rmpath)
                continue;

            IGV u = get_v_ig(s, vmg);
            for (IGE e : out_edges(u, ig)) {
                IGV v = target(e, ig);

                // skip edges in MinedGraph
                if (get_e_mg(s, e) != MGE())
                    continue;

                // u vertex exists in rmpath

                if (get_v_mg(s, v) == MGV()) {
                    // v vertex does not exist in MinedGraph
                    // R forward

                    auto src = v_index(mg, vmg);
                    auto dst = v_index(mg, rmost_v) + 1;
                    add_edge(r_ext, src, dst, &mg, e, &s, vpt, ept);
                }
                else {
                    // v vertex exists in MinedGraph

                    if (rmpath_vi_mask[v_index(mg, get_v_mg(s, v))]) {

                        if (is_rmost) {
                            // u vertex is rmost
                            // v vertex exists in rmpath
                            // R backward

                            auto src = v_index(mg, vmg);
                            auto dst = v_index(mg, get_v_mg(s, v));
                            add_edge(r_ext, src, dst, &mg, e, &s, vpt, ept);
                        }
                    }
                }
            } // for all out_edges
        } // for all vertices in MinedGraph
    } // for all SBG list
}

template <typename RExt, typename IG, typename VPT, typename EPT>
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
        : vptag_(vptag), eptag_(eptag), minsup_(minsup), result_(result)
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
    if (!is_minimum(mg)) {
        return;
    }

    result_(mg, sg, supp);

    RExt r_edges;
    XExt x_edges;
    for (const auto& x : sg)
        enumerate(r_edges, mg, *x.first, x.second, vptag_, eptag_);

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
gspan_many_graphs(IGIter ig_begin,
                  IGIter ig_end,
                  unsigned int minsup,
                  Result result,
                  VPTag vptag,
                  EPTag eptag)
{
    using IG = typename std::iterator_traits<IGIter>::value_type;
    typename gspan_traits<IG, VPTag, EPTag>::RExt r_ext;
    for (; ig_begin != ig_end; ++ig_begin) {
        gspan::enumerate_one_edges(r_ext, &*ig_begin, vptag, eptag);
    }

    using Alg = gspan::Alg<IG, Result, gspan::many_graphs_tag, VPTag, EPTag>;
    Alg alg(result, minsup, vptag, eptag);
    alg.run(r_ext);
}

#endif
