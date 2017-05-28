INCLUDE = ../include
CXXFLAGS += -O0 -g -Wall -I$(INCLUDE) 

#-Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-local-typedefs

all: gspan test_minimum_check

GSPAN_HEADERS := \
$(INCLUDE)/gspan_edgecode_tree.hpp \
$(INCLUDE)/gspan_edgecode_compare.hpp \
$(INCLUDE)/gspan_subgraph_tree.hpp \
$(INCLUDE)/gspan_subgraph_lists.hpp \
$(INCLUDE)/gspan_types.hpp \
$(INCLUDE)/gspan_helpers.hpp \
$(INCLUDE)/gspan_minimum_check.hpp \
$(INCLUDE)/gspan.hpp

gspan: test_gspan.cpp ${GSPAN_HEADERS}
	$(CXX) $(CXXFLAGS) $< -o $@

test_minimum_check: test_minimum_check.cpp ${GSPAN_HEADERS}
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf *.o gspan test_minimum_check

