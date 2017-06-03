#ifndef GSPAN_SUBGRAPH_LISTS_HPP
#define GSPAN_SUBGRAPH_LISTS_HPP

#include <list>
#include <vector>

namespace gspan {

template <typename S>
class subgraph_lists {
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
    insert(Args&& ... args);
};

template <typename S>
template <class ... Args>
void
subgraph_lists<S>::insert(Args&& ... args)
{
    const S* s = &*all_list.emplace(all_list.begin(), args...);
    const auto ri_end = aut_list.rend();
    for (auto ri = aut_list.rbegin(); ri != ri_end; ++ri) {
        if (is_automorphic(s, *ri->begin())) {
            ri->push_back(s);
            return;
        }
    }
    aut_list.push_back(std::vector<const S*>({s}));
    ++aut_list_size;
}

} // namespace gspan

#endif
