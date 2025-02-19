CXX = g++
CXXFLAGS = -std=c++17 -pthread
LDFLAGS =

SRCS = benches/dedup.cpp benches/io-lat-write.cpp

all: shell bench bench_opt multi_proc

shell: shell.cpp
	$(CXX) $(CXXFLAGS) -o shell shell.cpp $(LDFLAGS)

bench: bench.cpp $(SRCS)
	$(CXX) $(CXXFLAGS) -o bench bench.cpp $(SRCS) $(LDFLAGS)

bench_opt: bench.cpp $(SRCS)
	$(CXX) $(CXXFLAGS) -Ofast -o bench_opt bench.cpp $(SRCS) $(LDFLAGS)

multi_proc: multi_proc.cpp
	$(CXX) $(CXXFLAGS) -o multi_proc multi_proc.cpp $(LDFLAGS)

clean:
	rm -f shell bench bench_opt multi_proc
