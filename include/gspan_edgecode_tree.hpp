/**
 * \file
 * \author stvdedal@gmail.com
 *
 * \brief
 * Edge code and dfs code
 */
#ifndef GSPAN_EDGECODE_TREE_HPP
#define GSPAN_EDGECODE_TREE_HPP

#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>

#include <boost/call_traits.hpp>
#include <boost/assert.hpp>

#include <limits>
#include <utility>
#include <iterator>
#include <iostream> // for debug
#include <vector>   // for print_dfsc()

namespace gspan {

  // ==========================================================================
  // class edgecodetree_pmap

  namespace detail {
    template <typename PropertyTag, typename Graph>
    struct edgecodetree_pmap_impl;

    // specialized pmap for index properties accessed via *_index_t tag

    template <typename Graph>
    struct edgecodetree_pmap_impl<boost::vertex_index_t, Graph>
    {
      typedef typename Graph::vertex_descriptor_reference key_type;
      typedef typename Graph::vertex_index_type value_type;
      typedef typename Graph::vertex_index_type reference;
      static reference
      get_(key_type key, const Graph& g, boost::vertex_index_t)
      {
        return g.vertex_index(key);
      }
    };

    template <typename Graph>
    struct edgecodetree_pmap_impl<boost::edge_index_t, Graph>
    {
      typedef typename Graph::edge_descriptor_reference key_type;
      typedef typename Graph::edge_index_type value_type;
      typedef typename Graph::edge_index_type reference;
      static reference
      get_(key_type key, const Graph& g, boost::edge_index_t)
      {
        return g.edge_index(key);
      }
    };

    // specialized pmap for bundled properties accessed via *_bundle_t tag

    template <typename Graph>
    struct edgecodetree_pmap_impl<boost::vertex_bundle_t, Graph>
    {
      typedef typename Graph::vertex_descriptor_reference key_type;
      typedef typename Graph::vertex_bundled_type value_type;
      typedef typename Graph::vertex_bundled_reference reference;
      static reference
      get_(key_type key, const Graph& g, boost::vertex_bundle_t)
      {
        return g.vertex_value(key);
      }
    };

    template <typename Graph>
    struct edgecodetree_pmap_impl<boost::edge_bundle_t, Graph>
    {
      typedef typename Graph::edge_descriptor_reference key_type;
      typedef typename Graph::edge_bundled_type value_type;
      typedef typename Graph::edge_bundled_reference reference;
      static reference
      get_(key_type key, const Graph& g, boost::edge_bundle_t)
      {
        return g.edge_value(key);
      }
    };

    // specialized pmap for bundled properties accessed via pointer to member

    template <typename T, typename Graph>
    struct edgecodetree_pmap_impl<T Graph::vertex_bundled_type::*, Graph>
    {
      typedef typename Graph::vertex_descriptor_reference key_type;
      typedef T value_type;
      typedef const value_type& reference;
      static reference
      get_(key_type key,
           const Graph& g,
           value_type Graph::vertex_bundled_type::*pm)
      {
        return g.vertex_value(key).*pm;
      }
    };

    template <typename T, typename Graph>
    struct edgecodetree_pmap_impl<T Graph::edge_bundled_type::*, Graph>
    {
      typedef typename Graph::edge_descriptor_reference key_type;
      typedef T value_type;
      typedef const value_type& reference;
      static reference
      get_(key_type key,
           const Graph& g,
           value_type Graph::edge_bundled_type::*pm)
      {
        return g.edge_value(key).*pm;
      }
    };

  } // namespace detail

  template <typename PropertyTag, typename Graph>
  class edgecodetree_pmap
  {
    typedef detail::edgecodetree_pmap_impl<PropertyTag, Graph> Impl;
  public:
    edgecodetree_pmap(const Graph& g, PropertyTag tag)
      : _g(g), _tag(tag)
    {
    }
    typedef typename Impl::key_type key_type;
    typedef typename Impl::value_type value_type;
    typedef typename Impl::reference reference;
    typedef boost::readable_property_map_tag category;
    reference
    operator[](key_type k) const
    {
      return Impl::get_(k, _g, _tag);
    }
  private:
    const Graph& _g;
    PropertyTag _tag;
  };

  template <typename PropertyTag, typename Graph>
  typename edgecodetree_pmap<PropertyTag, Graph>::reference
  get(edgecodetree_pmap<PropertyTag, Graph> pmap,
      typename edgecodetree_pmap<PropertyTag, Graph>::key_type key)
  {
    return pmap[key];
  }

  // ==========================================================================
  // class edgecodetree

  // traversal_category
  struct edgecodetree_graph_tag : public boost::edge_list_graph_tag,
                                  public boost::vertex_list_graph_tag,
                                  public boost::incidence_graph_tag
  {
  };

  /**
   * \brief
   * Graph based on single-linked list.
   * It represents edge code and dfs code
   */
  template <typename VP, typename EP, typename D = boost::directedS,
    typename VI = std::size_t, typename EI = std::size_t>
  class edgecodetree
  {
  public:
    typedef VP vertex_bundled_type;
    typedef typename boost::call_traits<vertex_bundled_type>::param_type vertex_bundled_reference;

    typedef EP edge_bundled_type;
    typedef typename boost::call_traits<edge_bundled_type>::param_type edge_bundled_reference;

    typedef VI vertex_index_type;
    typedef EI edge_index_type;

    edgecodetree(vertex_index_type src,
                 vertex_index_type dst,
                 vertex_bundled_reference src_bundle,
                 vertex_bundled_reference dst_bundle,
                 edge_bundled_reference edge_bundle,
                 const edgecodetree* prev = nullptr);

    edgecodetree(const edgecodetree&) = delete;
    edgecodetree(edgecodetree&&);

    inline bool
    is_forward() const;

    inline const edgecodetree*
    prev() const;

    inline const edgecodetree*
    rmost() const;

    inline const edgecodetree*
    prev_rmost() const;

    // ------------------------------------------
    /// @name Graph concept requirements
    // ------------------------------------------
    ///@{

    class vertex_descriptor
    {
      friend class edgecodetree;
      friend class vertex_iterator;
    public:
      vertex_descriptor();
      inline
      vertex_descriptor(vertex_index_type vindex);
      inline bool
      operator==(const vertex_descriptor& rhs) const;
      inline bool
      operator!=(const vertex_descriptor& rhs) const;
    private:
      vertex_index_type _index;
    };

    class edge_descriptor
    {
      friend class edgecodetree;
    public:
      edge_descriptor();
      inline
      edge_descriptor(const edgecodetree* ec, bool direct);
      inline bool
      operator==(const edge_descriptor& rhs) const;
      inline bool
      operator!=(const edge_descriptor& rhs) const;
    private:
      const edgecodetree* _ec;
      bool _direct;
    };

    typedef typename boost::call_traits<vertex_descriptor>::param_type vertex_descriptor_reference;
    typedef typename boost::call_traits<edge_descriptor>::param_type edge_descriptor_reference;

    using directed_category = D;
    using edge_parallel_category = boost::allow_parallel_edge_tag;
    using traversal_category = edgecodetree_graph_tag;

    static vertex_descriptor
    null_vertex();

    ///@}
    // ------------------------------------------
    /// @name IncidenceGraph concept requirements
    // ------------------------------------------
    ///@{

    typedef edge_index_type degree_size_type;

    class out_edge_iterator : public std::iterator<std::forward_iterator_tag,
      edge_descriptor, degree_size_type>
    {
    public:
      out_edge_iterator();
      out_edge_iterator(const edgecodetree* ec, vertex_descriptor_reference v);

      edge_descriptor
      operator*() const;

      out_edge_iterator&
      operator++();

      out_edge_iterator
      operator++(int);

      inline bool
      operator==(const out_edge_iterator& rhs) const;

      inline bool
      operator!=(const out_edge_iterator& rhs) const;
    private:
      const edgecodetree* _ec;
      vertex_descriptor _v;

      static const edgecodetree*
      find(const edgecodetree* ec,
           vertex_descriptor_reference v,
           boost::directed_tag);
      static const edgecodetree*
      find(const edgecodetree* ec,
           vertex_descriptor_reference v,
           boost::undirected_tag);
    }; // class out_edge_iterator

    using out_edge_iterator_pair = std::pair<out_edge_iterator, out_edge_iterator>;

    inline out_edge_iterator_pair
    out_edges(vertex_descriptor_reference v) const;
    inline vertex_descriptor
    source(edge_descriptor_reference e) const;
    inline vertex_descriptor
    target(edge_descriptor_reference e) const;
    inline degree_size_type
    out_degree(vertex_descriptor_reference v) const;

    inline bool
    is_incident(vertex_descriptor_reference v) const;

    ///@}
    // ------------------------------------------
    /// @name VertexListGraph concept requirements
    // ------------------------------------------
    ///@{

    typedef vertex_index_type vertices_size_type;

    class vertex_iterator : public std::iterator<
      std::random_access_iterator_tag, vertex_descriptor, vertices_size_type>
    {
    public:
      vertex_iterator(vertex_descriptor v = vertex_descriptor());

      vertex_descriptor
      operator*() const;

      vertex_iterator&
      operator++();

      vertex_iterator
      operator++(int);

      inline bool
      operator==(const vertex_iterator& rhs) const;

      inline bool
      operator!=(const vertex_iterator& rhs) const;
    private:
      vertex_descriptor v_;
    };

    using vertex_iterator_pair = std::pair<vertex_iterator, vertex_iterator>;

    vertex_iterator_pair
    vertices() const;

    vertices_size_type
    num_vertices() const;

    ///@}
    // ------------------------------------------
    /// @name EdgeListGraph concept requirements
    // ------------------------------------------
    ///@{

    typedef edge_index_type edges_size_type;

    template <const edgecodetree*
    (edgecodetree::*pfm)() const>
    class edge_iterator_ : public std::iterator<std::forward_iterator_tag,
      edge_descriptor, edges_size_type>
    {
    public:
      edge_iterator_(const edgecodetree* ec = nullptr);

      inline edge_descriptor
      operator*() const;

      inline edge_iterator_&
      operator++();

      inline edge_iterator_
      operator++(int);

      inline bool
      operator==(const edge_iterator_& rhs) const;

      inline bool
      operator!=(const edge_iterator_& rhs) const;
    private:
      const edgecodetree* _ec;
    };
    typedef edge_iterator_<&edgecodetree::prev> edge_iterator;
    typedef std::pair<edge_iterator, edge_iterator> edge_iterator_pair;

    inline edge_iterator_pair
    edges() const;
    inline edges_size_type
    num_edges() const;

    typedef edge_iterator_<&edgecodetree::prev_rmost> rmpath_edge_iterator;
    typedef std::pair<rmpath_edge_iterator, rmpath_edge_iterator> rmpath_edge_iterator_pair;

    rmpath_edge_iterator_pair
    rmpath_edges() const;
    edges_size_type
    num_rmpath_edges() const;

    ///@}
    // ------------------------------------------
    /// @name Property map concept support
    // ------------------------------------------
    ///@{

    typedef vertex_bundled_type vertex_bundled;
    typedef edge_bundled_type edge_bundled;

    inline vertex_index_type
    vertex_index(vertex_descriptor_reference k) const;

    inline vertex_bundled_reference
    vertex_value(vertex_descriptor_reference k) const;

    inline edge_index_type
    edge_index(edge_descriptor_reference k) const;

    inline edge_bundled_reference
    edge_value(edge_descriptor_reference k) const;

    ///@}

  private:

    vertex_index_type _src_vindex;
    vertex_index_type _dst_vindex;
    edge_index_type _eindex;

    vertex_bundled_type _src_bundled;
    vertex_bundled_type _dst_bundled;
    edge_bundled_type _edge_bundled;

    const edgecodetree* _prev;
    const edgecodetree* _rmost;
    const edgecodetree* _prev_rmost;
    const edgecodetree* _prev_src;
    const edgecodetree* _prev_dst;

    inline bool
    is_incident(vertex_descriptor_reference v, boost::directed_tag) const;
    inline bool
    is_incident(vertex_descriptor_reference v, boost::undirected_tag) const;
    const edgecodetree*
    find_incident(vertex_descriptor v) const;

    template <typename VP_, typename EP_, typename D_, typename VI_,
      typename EI_>
    friend std::ostream&
    operator<<(std::ostream& s, const edgecodetree<VP_, EP_, D_, VI_, EI_>& ec);
  };

  // ==========================================================================
  // class edgecodetree::vertex_descriptor

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor::vertex_descriptor()
    : _index(std::numeric_limits<vertex_index_type>::max())
  {
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor::vertex_descriptor(vertex_index_type vindex)
    : _index(vindex)
  {
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor::operator==(const vertex_descriptor& rhs) const
  {
    return _index == rhs._index;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  inline bool
  edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor::operator!=(const vertex_descriptor& rhs) const
  {
    return !(*this == rhs);
  }

  // ==========================================================================
  // class edgecodetree::edge_descriptor

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  edgecodetree<VP, EP, D, VI, EI>::edge_descriptor::edge_descriptor()
    : _ec(nullptr), _direct()
  {
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  edgecodetree<VP, EP, D, VI, EI>::edge_descriptor::edge_descriptor(const edgecodetree* ec,
                                                                    bool direct)
    : _ec(ec), _direct(direct)
  {
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::edge_descriptor::operator==(const edge_descriptor& rhs) const
  {
    return _ec == rhs._ec;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::edge_descriptor::operator!=(const edge_descriptor& rhs) const
  {
    return !(*this == rhs);
  }

  // ==========================================================================
  // class edgecodetree::out_edge_iterator

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator::out_edge_iterator()
    : _ec(nullptr), _v()
  {
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator::out_edge_iterator(const edgecodetree* ec,
                                                                        vertex_descriptor_reference v)
    : _ec(nullptr), _v(v)
  {
    if (ec && ec->is_incident(v))
      _ec = ec;
    else
      _ec = find(ec, v, directed_category());
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::edge_descriptor
  edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator::operator*() const
  {
    BOOST_ASSERT(!_ec || _ec->is_incident(_v));
    return edge_descriptor(_ec, _ec->_src_vindex == _v._index);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator&
  edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator::operator++()
  {
    _ec = find(_ec, _v, directed_category());
    return *this;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator
  edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator::operator++(int)
  {
    edgecodetree copy(*this);
    ++*this;
    return copy;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator::operator==(const out_edge_iterator& rhs) const
  {
    return _ec == rhs._ec && _v == rhs._v;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator::operator!=(const out_edge_iterator& rhs) const
  {
    return !(*this == rhs);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  const edgecodetree<VP, EP, D, VI, EI>*
  edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator::find(const edgecodetree* ec,
                                                           vertex_descriptor_reference v,
                                                           boost::directed_tag)
  {
    if (!ec)
      return nullptr;
    if (ec->_src_vindex == v._index)
      return ec->_prev_src;
    for (const edgecodetree* p = ec->prev(); p; p = p->prev()) {
      if (p->is_incident(v))
        return p;
    }
    return nullptr;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  const edgecodetree<VP, EP, D, VI, EI>*
  edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator::find(const edgecodetree* ec,
                                                           vertex_descriptor_reference v,
                                                           boost::undirected_tag)
  {
    if (!ec)
      return nullptr;
    if (ec->_src_vindex == v._index)
      return ec->_prev_src;
    if (ec->_dst_vindex == v._index)
      return ec->_prev_dst;
    for (const edgecodetree* p = ec->prev(); p; p = p->prev()) {
      if (p->is_incident(v))
        return p;
    }
    return nullptr;
  }

  // ==========================================================================
  // class edgecodetree::vertex_iterator

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  edgecodetree<VP, EP, D, VI, EI>::vertex_iterator::vertex_iterator(vertex_descriptor v)
    : v_(v)
  {
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor
  edgecodetree<VP, EP, D, VI, EI>::vertex_iterator::operator*() const
  {
    return v_;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_iterator&
  edgecodetree<VP, EP, D, VI, EI>::vertex_iterator::operator++()
  {
    ++v_._index;
    return *this;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_iterator
  edgecodetree<VP, EP, D, VI, EI>::vertex_iterator::operator++(int)
  {
    vertex_iterator copy(*this);
    ++v_._index;
    return copy;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::vertex_iterator::operator==(const vertex_iterator& rhs) const
  {
    return v_ == rhs.v_;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::vertex_iterator::operator!=(const vertex_iterator& rhs) const
  {
    return !(*this == rhs);
  }

  // ==========================================================================
  // class edgecodetree::edge_iterator_

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  template <const edgecodetree<VP, EP, D, VI, EI>*
  (edgecodetree<VP, EP, D, VI, EI>::*pfm)() const>
  edgecodetree<VP, EP, D, VI, EI>::edge_iterator_<pfm>::edge_iterator_(const edgecodetree* ec)
    : _ec(ec)
  {
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  template <const edgecodetree<VP, EP, D, VI, EI>*
  (edgecodetree<VP, EP, D, VI, EI>::*pfm)() const>
  typename edgecodetree<VP, EP, D, VI, EI>::edge_descriptor
  edgecodetree<VP, EP, D, VI, EI>::edge_iterator_<pfm>::operator*() const
  {
    return edge_descriptor(_ec, true);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  template <const edgecodetree<VP, EP, D, VI, EI>*
  (edgecodetree<VP, EP, D, VI, EI>::*pfm)() const>
  typename edgecodetree<VP, EP, D, VI, EI>::template edge_iterator_<pfm>&
  edgecodetree<VP, EP, D, VI, EI>::edge_iterator_<pfm>::operator++()
  {
    _ec = (_ec->*pfm)();
    return *this;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  template <const edgecodetree<VP, EP, D, VI, EI>*
  (edgecodetree<VP, EP, D, VI, EI>::*pfm)() const>
  typename edgecodetree<VP, EP, D, VI, EI>::template edge_iterator_<pfm>
  edgecodetree<VP, EP, D, VI, EI>::edge_iterator_<pfm>::operator++(int)
  {
    edge_iterator_ copy(*this);
    ++*this;
    return copy;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  template <const edgecodetree<VP, EP, D, VI, EI>*
  (edgecodetree<VP, EP, D, VI, EI>::*pfm)() const>
  bool
  edgecodetree<VP, EP, D, VI, EI>::edge_iterator_<pfm>::operator==(const edge_iterator_& rhs) const
  {
    return _ec == rhs._ec;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  template <const edgecodetree<VP, EP, D, VI, EI>*
  (edgecodetree<VP, EP, D, VI, EI>::*pfm)() const>
  bool
  edgecodetree<VP, EP, D, VI, EI>::edge_iterator_<pfm>::operator!=(const edge_iterator_& rhs) const
  {
    return !(*this == rhs);
  }

  // ==========================================================================
  // class edgecodetree

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  edgecodetree<VP, EP, D, VI, EI>::edgecodetree(vertex_index_type src,
                                                vertex_index_type dst,
                                                vertex_bundled_reference src_bundle,
                                                vertex_bundled_reference dst_bundle,
                                                edge_bundled_reference edge_bundle,
                                                const edgecodetree* prev)
    : _src_vindex(src), _dst_vindex(dst), _eindex(0), _src_bundled(src_bundle),
      _dst_bundled(dst_bundle), _edge_bundled(edge_bundle), _prev(prev),
      _rmost(nullptr), _prev_rmost(nullptr), _prev_src(nullptr),
      _prev_dst(nullptr)
  {
    _rmost = is_forward() ? this : _prev ? _prev->rmost() : nullptr;
    _eindex = prev ? prev->_eindex + 1 : 0;

    for (const edgecodetree* p = prev; p; p = p->prev()) {
      if (_prev_rmost && _prev_src && _prev_dst)
        break;
      if (!_prev_rmost && p->is_forward() && src == p->_dst_vindex) {
        _prev_rmost = p;
      }
      if (!_prev_src && p->is_incident(src))
        _prev_src = p;
      if (!_prev_dst && p->is_incident(dst))
        _prev_dst = p;
    }
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  edgecodetree<VP, EP, D, VI, EI>::edgecodetree(edgecodetree&& rhs)
    : _src_vindex(std::move(rhs._src_vindex)),
      _dst_vindex(std::move(rhs._dst_vindex)), _eindex(std::move(rhs._eindex)),
      _src_bundled(std::move(rhs._src_bundled)),
      _dst_bundled(std::move(rhs._dst_bundled)),
      _edge_bundled(std::move(rhs._edge_bundled)), _prev(std::move(rhs._prev)),
      _rmost(std::move(rhs._rmost)), _prev_rmost(std::move(rhs._prev_rmost)),
      _prev_src(std::move(rhs._prev_src)), _prev_dst(std::move(rhs._prev_dst))
  {
    _rmost = is_forward() ? this : _prev ? _prev->rmost() : nullptr;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::is_forward() const
  {
    return _src_vindex < _dst_vindex;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  const edgecodetree<VP, EP, D, VI, EI>*
  edgecodetree<VP, EP, D, VI, EI>::prev() const
  {
    return _prev;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  const edgecodetree<VP, EP, D, VI, EI>*
  edgecodetree<VP, EP, D, VI, EI>::rmost() const
  {
    return _rmost;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  const edgecodetree<VP, EP, D, VI, EI>*
  edgecodetree<VP, EP, D, VI, EI>::prev_rmost() const
  {
    return _prev_rmost;
  }

  // ------------------------------------------
  // Graph requirements
  // ------------------------------------------

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor
  edgecodetree<VP, EP, D, VI, EI>::null_vertex()
  {
    return std::numeric_limits<vertex_descriptor>::max();
  }

  // ------------------------------------------
  // IncidenceGraph requirements
  // ------------------------------------------

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator_pair
  edgecodetree<VP, EP, D, VI, EI>::out_edges(vertex_descriptor_reference v) const
  {
    return out_edge_iterator_pair(out_edge_iterator(this, v),
                                  out_edge_iterator(nullptr, v));
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::out_edge_iterator_pair
  out_edges(typename edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor_reference v,
            const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.out_edges(v);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor
  edgecodetree<VP, EP, D, VI, EI>::source(edge_descriptor_reference e) const
  {
    return vertex_descriptor(
      e._direct ? e._ec->_src_vindex : e._ec->_dst_vindex);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor
  source(typename edgecodetree<VP, EP, D, VI, EI>::edge_descriptor_reference e,
         const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.source(e);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor
  edgecodetree<VP, EP, D, VI, EI>::target(edge_descriptor_reference e) const
  {
    return vertex_descriptor(
      e._direct ? e._ec->_dst_vindex : e._ec->_src_vindex);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor
  target(typename edgecodetree<VP, EP, D, VI, EI>::edge_descriptor_reference e,
         const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.target(e);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::degree_size_type
  edgecodetree<VP, EP, D, VI, EI>::out_degree(vertex_descriptor_reference v) const
  {
    out_edge_iterator_pair p = out_edges(v);
    return std::distance(p.first, p.second);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::degree_size_type
  out_degree(typename edgecodetree<VP, EP, D, VI, EI>::vertex_descriptor_reference v,
             const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.out_degree(v);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::is_incident(vertex_descriptor_reference v) const
  {
    return is_incident(v, directed_category());
  }

  // ------------------------------------------
  // VertexListGraph requirements
  // ------------------------------------------

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_iterator_pair
  edgecodetree<VP, EP, D, VI, EI>::vertices() const
  {
    return vertex_iterator_pair(vertex_iterator(0),
                                vertex_iterator(num_vertices()));
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_iterator_pair
  vertices(const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.vertices();
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertices_size_type
  edgecodetree<VP, EP, D, VI, EI>::num_vertices() const
  {
    return rmost()->_dst_vindex + 1;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertices_size_type
  num_vertices(const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.num_vertices();
  }

  // ------------------------------------------
  // EdgeListGraph requirements
  // ------------------------------------------

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::edge_iterator_pair
  edgecodetree<VP, EP, D, VI, EI>::edges() const
  {
    return edge_iterator_pair(edge_iterator(this), edge_iterator(nullptr));
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::edge_iterator_pair
  edges(const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.edges();
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::edges_size_type
  edgecodetree<VP, EP, D, VI, EI>::num_edges() const
  {
    return _eindex + 1;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::edges_size_type
  num_edges(const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.num_edges();
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::rmpath_edge_iterator_pair
  edgecodetree<VP, EP, D, VI, EI>::rmpath_edges() const
  {
    return rmpath_edge_iterator_pair(rmpath_edge_iterator(rmost()),
                                     rmpath_edge_iterator(nullptr));
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::rmpath_edge_iterator_pair
  rmpath_edges(const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.rmpath_edges();
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::edges_size_type
  edgecodetree<VP, EP, D, VI, EI>::num_rmpath_edges() const
  {
    rmpath_edge_iterator_pair p = rmpath_edges();
    return std::distance(p.first, p.second);
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::edges_size_type
  num_rmpath_edges(const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return g.num_rmpath_edges();
  }

  // ------------------------------------------
  // Property map support
  // ------------------------------------------

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_index_type
  edgecodetree<VP, EP, D, VI, EI>::vertex_index(vertex_descriptor_reference v) const
  {
    return v._index;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::vertex_bundled_reference
  edgecodetree<VP, EP, D, VI, EI>::vertex_value(vertex_descriptor_reference v) const
  {
    const edgecodetree* ec = find_incident(v);
    if (!ec) {
      static vertex_bundled_type nil;
      return nil;
    }
    return ec->_src_vindex == v._index ? ec->_src_bundled : ec->_dst_bundled;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::edge_index_type
  edgecodetree<VP, EP, D, VI, EI>::edge_index(edge_descriptor_reference e) const
  {
    if (e._ec)
      return e._ec->_eindex;
    return std::numeric_limits<edge_index_type>::max();
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  typename edgecodetree<VP, EP, D, VI, EI>::edge_bundled_reference
  edgecodetree<VP, EP, D, VI, EI>::edge_value(edge_descriptor_reference e) const
  {
    if (!e._ec) {
      static edge_bundled_type nil;
      return nil;
    }
    return e._ec->_edge_bundled;
  }

  // ------------------------------------------
  // privates
  // ------------------------------------------

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::is_incident(vertex_descriptor_reference v,
                                               boost::directed_tag) const
  {
    return _src_vindex == v._index;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  bool
  edgecodetree<VP, EP, D, VI, EI>::is_incident(vertex_descriptor_reference v,
                                               boost::undirected_tag) const
  {
    return _src_vindex == v._index || _dst_vindex == v._index;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  const edgecodetree<VP, EP, D, VI, EI>*
  edgecodetree<VP, EP, D, VI, EI>::find_incident(vertex_descriptor v) const
  {
    for (const edgecodetree* p = this; p; p = p->prev())
      if (p->is_incident(v))
        return p;
    return nullptr;
  }

}
// namespace gspan

namespace boost {
  // ------------------------------------------
  // PropertyGraph requirements
  // ------------------------------------------
  template <typename VP, typename EP, typename D, typename VI, typename EI,
    typename PropertyTag>
  class property_map<gspan::edgecodetree<VP, EP, D, VI, EI>, PropertyTag>
  {
    typedef gspan::edgecodetree<VP, EP, D, VI, EI> G;
  public:
    typedef gspan::edgecodetree_pmap<PropertyTag, G> type;
    typedef gspan::edgecodetree_pmap<PropertyTag, G> const_type;
  };
} // namespace boost
namespace gspan {

  //
  // get(p, g)
  //
  template <typename VP, typename EP, typename D, typename VI, typename EI,
    typename PropertyTag>
  typename boost::property_map<edgecodetree<VP, EP, D, VI, EI>, PropertyTag>::const_type
  get(PropertyTag p, const edgecodetree<VP, EP, D, VI, EI>& g)
  {
    return typename boost::property_map<edgecodetree<VP, EP, D, VI, EI>,
      PropertyTag>::const_type(g, p);
  }

  //
  // get(p, g, x)
  //
  template <typename VP, typename EP, typename D, typename VI, typename EI,
    typename PropertyTag, typename X>
  typename boost::property_traits<
    typename boost::property_map<edgecodetree<VP, EP, D, VI, EI>, PropertyTag>::const_type>::reference
  get(PropertyTag p, const edgecodetree<VP, EP, D, VI, EI>& g, X&& x)
  {
    return get(get(p, g), x);
  }
} // namespace gspan

namespace gspan {
  // ==========================================================================
  // debug

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  std::ostream&
  operator<<(std::ostream& s, const edgecodetree<VP, EP, D, VI, EI>& ec)
  {
    s << "(" << ec._src_vindex << "," << ec._dst_vindex << "," << ec._eindex
    << ", " << ec._src_bundled << ", " << ec._dst_bundled << ", "
    << ec._edge_bundled << ") at " << &ec;
    return s;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  std::ostream&
  print_edge(const typename edgecodetree<VP, EP, D, VI, EI>::edge_descriptor& e,
             const edgecodetree<VP, EP, D, VI, EI>& g,
             std::ostream& s = std::cerr)
  {
    using G = edgecodetree<VP, EP, D, VI, EI>;

    typename boost::property_map<G, boost::vertex_index_t>::const_type vindex =
      get(boost::vertex_index_t(), g);
    typename boost::property_map<G, boost::vertex_bundle_t>::const_type vvalue =
      get(boost::vertex_bundle_t(), g);
    typename boost::property_map<G, boost::edge_bundle_t>::const_type evalue =
      get(boost::edge_bundle_t(), g);

    typename G::vertex_descriptor u = source(e, g);
    typename G::vertex_descriptor v = target(e, g);

    s << "(";
    s << get(vindex, u) << "," << get(vindex, v);
    s << ", ";
    s << get(vvalue, u) << "," << get(evalue, e) << "," << get(vvalue, v);
    s << ")";

    return s;
  }

  template <typename VP, typename EP, typename D, typename VI, typename EI>
  std::ostream&
  print_dfsc(const edgecodetree<VP, EP, D, VI, EI>& g, std::ostream& s =
    std::cerr)
  {
    typedef edgecodetree<VP, EP, D, VI, EI> G;
    typename boost::property_map<G, boost::edge_index_t>::const_type eindex =
      get(boost::edge_index_t(), g);

    std::vector<bool> rmpath(num_edges(g), false);
    std::vector<typename G::edge_descriptor> dfscode;

    for (typename G::rmpath_edge_iterator_pair p = rmpath_edges(g);
      p.first != p.second; ++p.first) {
      rmpath[get(eindex, *p.first)] = true;
    }

    for (typename G::edge_iterator_pair p = edges(g); p.first != p.second;
      ++p.first) {
      dfscode.push_back(*p.first);
    }

    for (typename std::vector<typename G::edge_descriptor>::const_reverse_iterator ri =
      dfscode.rbegin(); ri != dfscode.rend(); ++ri) {
      typename G::edge_descriptor e = *ri;
      s << (rmpath.at(get(eindex, e)) ? " * " : "   ");
      print_edge(e, g, s);
      s << std::endl;
    }
    return s;
  }

  // ==========================================================================
  // Helpers

  template <typename Ec, typename V>
  auto
  v_index(const Ec& ec, V&& v)
  {
    return get(boost::vertex_index_t(), ec, v);
  }

  template <typename Ec, typename E>
  auto
  e_index(const Ec& ec, E&& e)
  {
    return get(boost::edge_index_t(), ec, e);
  }

  template <typename Ec, typename V>
  auto
  v_bundle(const Ec& ec, V&& v)
  {
    return get(boost::vertex_bundle_t(), ec, v);
  }

  template <typename Ec, typename E>
  auto
  e_bundle(const Ec& ec, E&& e)
  {
    return get(boost::edge_bundle_t(), ec, e);
  }

  template <typename Ec, typename E>
  auto
  source_index(const Ec& ec, E&& e)
  {
    return v_index(ec, source(e, ec));
  }

  template <typename Ec, typename E>
  auto
  target_index(const Ec& ec, E&& e)
  {
    return v_index(ec, target(e, ec));
  }

  template <typename Ec, typename E>
  auto
  source_bundle(const Ec& ec, E&& e)
  {
    return v_bundle(ec, source(e, ec));
  }

  template <typename Ec, typename E>
  auto
  target_bundle(const Ec& ec, E&& e)
  {
    return v_bundle(ec, target(e, ec));
  }

  template <typename Ec, typename E>
  bool
  is_forward(const Ec& ec, E&& e)
  {
    return source_index(ec, e) < target_index(ec, e);
  }

} // namespace gspan

#endif /* GSPAN_EDGECODE_TREE_HPP */
