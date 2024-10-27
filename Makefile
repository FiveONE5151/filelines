.PHONY: clean all release debug

CXX = g++
CFLAGS = -Wall -pthread
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -g
TARGET_DIR = ./bin

targets = filelines filelines_gen testBlocked filelines_avx filelines_pc
objects = filelines.o find_most_freq.o filelines_baseline.o

all: release

release: CFLAGS += $(RELEASE_FLAGS)
release: $(targets)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(targets)

filelines: $(objects)
	$(CXX) $(CFLAGS) $(objects) -o $(TARGET_DIR)/$@

filelines_gen: filelines_gen.o find_most_freq.o
	$(CXX) $(CFLAGS) $^ -o $(TARGET_DIR)/$@

testBlocked: testBlocked.o find_most_freq.o 
	$(CXX) $(CFLAGS) $^ -o $(TARGET_DIR)/$@

filelines_avx: filelines_avx.o find_most_freq.o
	$(CXX) $(CFLAGS) $^ -o $(TARGET_DIR)/$@
filelines_pc: filelines_pc.o find_most_freq.o
	$(CXX) $(CFLAGS) $^ -o $(TARGET_DIR)/$@
%.o: %.cpp %.h
	$(CXX) $(CFLAGS) -c $< -o $@

filelines_gen.o: filelines_gen.cpp find_most_freq.h
	$(CXX) $(CFLAGS) -c $< -o $@

testBlocked.o: testBlocked.cpp find_most_freq.h TestResult.h
	$(CXX) $(CFLAGS) -c $< -o $@

filelines_avx.o: filelines_avx.cpp find_most_freq.h
	$(CXX) $(CFLAGS) -c $< -o $@
filelines_pc.o: filelines_pc.cpp find_most_freq.h
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(objects) filelines_gen.o testBlocked.o filelines_avx.o filelines_pc.o $(targets)