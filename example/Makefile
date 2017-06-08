INCLUDE := ../include
CXXFLAGS += -O3 -g -Wall -I$(INCLUDE) 
#CXXFLAGS += -DBOOST_DISABLE_ASSERTS 
#CXXFLAGS += -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-local-typedefs

GSPAN_HEADERS := $(shell find $(INCLUDE) -type f -name '*.hpp')

all: gspan

gspan: test_gspan.cpp Makefile $(GSPAN_HEADERS)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf *.o gspan
