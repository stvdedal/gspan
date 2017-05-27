#ifndef GSPAN_SUBGRAPH_LISTS_HPP
#define GSPAN_SUBGRAPH_LISTS_HPP

#include <list>
#include <vector>

namespace gspan {

  template <typename S>
  class subgraph_lists
  {
  public:
    subgraph_lists()
      : aut_list_size(0)
    {
    }

    subgraph_lists(const subgraph_lists&) = delete;
    subgraph_lists&
    operator=(const subgraph_lists&) = delete;
    subgraph_lists(subgraph_lists&& rhs) = default;

    std::list<S> all_list;
    std::list<std::vector<const S*>> aut_list;
    unsigned int aut_list_size;

    template <class ... Args>
    void
    insert(Args&&... args);
  };

  template <typename S>
  template <class ... Args>
  void
  subgraph_lists<S>::insert(Args&&... args)
  {
    const S* s = &*all_list.emplace(all_list.begin(), args...);

    bool aut_found = false;
    for (auto& group : aut_list) {
      if (is_automorphic(s, *group.begin())) {
        aut_found = true;
        group.push_back(s);
        break;
      }
    }
    if (!aut_found) {
      aut_list.resize(aut_list.size() + 1);
      aut_list.back().push_back(s);
      ++aut_list_size;
    }
  }

} // namespace gspan

#endif
