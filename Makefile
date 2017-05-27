CXXFLAGS := -O0 -fPIC -g -Wall -I./include

#-Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-local-typedefs

all: example/test_gspan example/test_minimum_check

GSPAN_HEADERS := \
./include/gspan_edgecode_tree.hpp \
./include/gspan_edgecode_compare.hpp \
./include/gspan_subgraph_tree.hpp \
./include/gspan_subgraph_lists.hpp \
./include/gspan_types.hpp \
./include/gspan_helpers.hpp \
./include/gspan_minimum_check.hpp \
./include/gspan.hpp

example/test_gspan: example/test_gspan.cpp ${GSPAN_HEADERS}
	$(CXX) $(CXXFLAGS) $@.cpp -o $@

example/test_minimum_check: example/test_minimum_check.cpp ${GSPAN_HEADERS}
	$(CXX) $(CXXFLAGS) $< -o $@
