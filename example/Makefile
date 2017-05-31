INCLUDE := ../include
CXXFLAGS += -O3 -Wall -I$(INCLUDE) -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-local-typedefs
GSPAN_HEADERS := $(shell find $(INCLUDE) -type f -name '*.hpp')

all: gspan test_minimum_check

gspan: test_gspan.cpp $(GSPAN_HEADERS)
	$(CXX) $(CXXFLAGS) $< -o $@

test_minimum_check: test_minimum_check.cpp $(GSPAN_HEADERS)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf *.o gspan test_minimum_check
