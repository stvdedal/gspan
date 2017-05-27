#ifndef GSPAN_HELPERS_HPP
#define GSPAN_HELPERS_HPP

#include "gspan_types.hpp"
#include <iterator>
#include <boost/range/adaptor/reversed.hpp>

namespace gspan {

  template <typename G, typename VPTag, typename EPTag>
  typename gspan_traits<G, VPTag, EPTag>::MG
  make_ec(typename gspan_traits<G, VPTag, EPTag>::VI vi_src,
          typename gspan_traits<G, VPTag, EPTag>::VI vi_dst,
          const typename gspan_traits<G, VPTag, EPTag>::MG* prev_ec,
          typename boost::graph_traits<G>::edge_descriptor e,
          const G& g,
          VPTag vptag,
          EPTag eptag)
  {
    using Ec = typename gspan_traits<G, VPTag, EPTag>::MG;
    return Ec(vi_src,
              vi_dst,
              get(vptag, g, source(e, g)),
              get(vptag, g, target(e, g)),
              get(eptag, g, e),
              prev_ec);
  }

} // namespace gspan

namespace std {
  template <typename Iter>
  Iter
  begin(const std::pair<Iter, Iter>& p)
  {
    return p.first;
  }

  template <typename Iter>
  Iter
  end(const std::pair<Iter, Iter>& p)
  {
    return p.second;
  }
}

#endif
