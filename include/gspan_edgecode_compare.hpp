/**
 * \file
 * \author stvdedal@gmail.com
 *
 * \brief
 * Edge code compare functors
 */
#ifndef GSPAN_EDGECODE_COMPARE_HPP
#define GSPAN_EDGECODE_COMPARE_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>

namespace gspan {

/// compare functors for R edge extentions
struct edgecode_compare_dfs {
    template <typename G>
    bool
    operator()(const G& lhs, const G& rhs) const;
};

/// compare functors for X edge extentions
struct edgecode_compare_lex {
    template <typename G>
    bool
    operator()(const G& lhs, const G& rhs) const;
};

template <typename G>
bool
edgecode_compare_dfs::operator()(const G& g1, const G& g2) const
{
    using boost::graph_traits;
    using boost::property_map;
    using boost::vertex_index_t;
    using boost::vertex_bundle_t;
    using boost::edge_index_t;
    using boost::edge_bundle_t;

    typedef graph_traits<G> Traits;
    typedef typename property_map<G, vertex_index_t>::const_type ViMap;
    typedef typename property_map<G, vertex_bundle_t>::const_type VvMap;
    typedef typename property_map<G, edge_bundle_t>::const_type EvMap;

    ViMap vi1 = get(vertex_index_t(), g1);
    VvMap vv1 = get(vertex_bundle_t(), g1);
    EvMap ev1 = get(edge_bundle_t(), g1);

    ViMap vi2 = get(vertex_index_t(), g2);
    VvMap vv2 = get(vertex_bundle_t(), g2);
    EvMap ev2 = get(edge_bundle_t(), g2);

    typename Traits::edge_descriptor e1 = *edges(g1).first;
    typename Traits::vertex_descriptor src1 = source(e1, g1);
    typename Traits::vertex_descriptor dst1 = target(e1, g1);

    typename Traits::edge_descriptor e2 = *edges(g2).first;
    typename Traits::vertex_descriptor src2 = source(e2, g2);
    typename Traits::vertex_descriptor dst2 = target(e2, g2);

    bool e1_is_forward = get(vi1, src1) < get(vi1, dst1);
    bool e2_is_forward = get(vi2, src2) < get(vi2, dst2);

    if (!e1_is_forward && e2_is_forward)
        return true;

    if (!e1_is_forward && !e2_is_forward) {
        if (get(vi1, dst1) < get(vi2, dst2))
            return true;
        else if (get(vi1, dst1) == get(vi2, dst2) && get(ev1, e1) < get(ev2, e2))
            return true;
        else
            return false;
    }

    if (e1_is_forward && e2_is_forward) {
        if (get(vi1, src1) > get(vi2, src2))
            return true;
        else if (get(vi1, src1) == get(vi2, src2)
                 && get(vv1, src1) < get(vv2, src2))
            return true;
        else if (get(vi1, src1) == get(vi2, src2)
                 && get(vv1, src1) == get(vv2, src2) && get(ev1, e1) < get(ev2, e2))
            return true;
        else if (get(vi1, src1) == get(vi2, src2)
                 && get(vv1, src1) == get(vv2, src2) && get(ev1, e1) == get(ev2, e2)
                 && get(vv1, dst1) < get(vv2, dst2))
            return true;
        else
            return false;
    }

    return false;
}

template <typename G>
bool
edgecode_compare_lex::operator()(const G& g1, const G& g2) const
{
    using boost::graph_traits;
    using boost::property_map;
    using boost::vertex_index_t;
    using boost::vertex_bundle_t;
    using boost::edge_index_t;
    using boost::edge_bundle_t;

    typedef graph_traits<G> Traits;
    typedef typename property_map<G, vertex_index_t>::const_type ViMap;
    typedef typename property_map<G, vertex_bundle_t>::const_type VvMap;
    typedef typename property_map<G, edge_index_t>::const_type EiMap;
    typedef typename property_map<G, edge_bundle_t>::const_type EvMap;

    ViMap vi1 = get(vertex_index_t(), g1);
    VvMap vv1 = get(vertex_bundle_t(), g1);
    EiMap ei1 = get(edge_index_t(), g1);
    EvMap ev1 = get(edge_bundle_t(), g1);

    ViMap vi2 = get(vertex_index_t(), g2);
    VvMap vv2 = get(vertex_bundle_t(), g2);
    EiMap ei2 = get(edge_index_t(), g2);
    EvMap ev2 = get(edge_bundle_t(), g2);

    typename Traits::edge_descriptor e1 = *edges(g1).first;
    typename Traits::vertex_descriptor src1 = source(e1, g1);
    typename Traits::vertex_descriptor dst1 = target(e1, g1);

    typename Traits::edge_descriptor e2 = *edges(g2).first;
    typename Traits::vertex_descriptor src2 = source(e2, g2);
    typename Traits::vertex_descriptor dst2 = target(e2, g2);

    if (get(vi1, src1) < get(vi2, src2))
        return true;
    if (get(vi1, src1) > get(vi2, src2))
        return false;

    if (get(vi1, dst1) < get(vi2, dst2))
        return true;
    if (get(vi1, dst1) > get(vi2, dst2))
        return false;

    if (get(vv1, src1) < get(vv2, src2))
        return true;
    if (get(vv1, src1) > get(vv2, src2))
        return false;

    if (get(ev1, e1) < get(ev2, e2))
        return true;
    if (get(ev1, e1) > get(ev2, e2))
        return false;

    if (get(vv1, dst1) < get(vv2, dst2))
        return true;

    return false;
}

} // namespace gspan

#endif
