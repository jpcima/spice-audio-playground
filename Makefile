CXX ?= g++
CXXFLAGS ?= -O2 -g

all: square-ng

clean:
	rm -f square-ng

square-ng: square-ng.cpp
	$(CXX) -o $@ $< `pkg-config ngspice --cflags --libs`

.PHONY: all clean
