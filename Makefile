.PHONY: clean all release debug

CXX = g++
CFLAGS = -Wall
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -g

targets = filelines filelines_gen testBlocked filelines_avx
objects = filelines.o find_most_freq.o filelines_baseline.o

all: release

release: CFLAGS += $(RELEASE_FLAGS)
release: $(targets)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(targets)

filelines: $(objects)
	$(CXX) $(CFLAGS) $(objects) -o $@

filelines_gen: filelines_gen.o find_most_freq.o
	$(CXX) $(CFLAGS) $^ -o $@

testBlocked: testBlocked.o find_most_freq.o 
	$(CXX) $(CFLAGS) $^ -o $@

filelines_avx: filelines_avx.o find_most_freq.o
	$(CXX) $(CFLAGS) $^ -o $@

%.o: %.cpp %.h
	$(CXX) $(CFLAGS) -c $< -o $@

filelines_gen.o: filelines_gen.cpp find_most_freq.h
	$(CXX) $(CFLAGS) -c $< -o $@

testBlocked.o: testBlocked.cpp find_most_freq.h TestResult.h
	$(CXX) $(CFLAGS) -c $< -o $@

filelines_avx.o: filelines_avx.cpp find_most_freq.h
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(objects) filelines_gen.o testBlocked.o filelines_avx.o $(targets)