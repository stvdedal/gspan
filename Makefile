CXXFLAGS := -O0 -fPIC -g -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-local-typedefs

all: test_gspan test_minimum_check

GSPAN_HEADERS := \
gspan_edgecode_tree.hpp \
gspan_edgecode_compare.hpp \
gspan_subgraph_tree.hpp \
gspan_subgraph_lists.hpp \
gspan_types.hpp \
gspan_helpers.hpp \
gspan_minimum_check.hpp \
gspan.hpp

test_gspan: test_gspan.cpp ${GSPAN_HEADERS}
	g++ ${CXXFLAGS} test_gspan.cpp -o test_gspan

test_minimum_check: test_minimum_check.cpp ${GSPAN_HEADERS}
	g++ ${CXXFLAGS} test_minimum_check.cpp -o test_minimum_check
