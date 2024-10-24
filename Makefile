.PHONY: clean all

CFLAGS = -Wall -g -O2
targets = filelines filelines_gen testBlocked filelines_avx
objects = filelines.o find_most_freq.o filelines_baseline.o

all: $(targets)
	echo "all done"
filelines: $(objects)
	g++ $(CFLAGS) $(objects) -o $@
filelines_gen: filelines_gen.o find_most_freq.o
	g++ $(CFLAGS) $^ -o $@
testBlocked: testBlocked.o find_most_freq.o 
	g++ $(CFLAGS) $^ -o $@
filelines_avx: filelines_avx.o find_most_freq.o
	g++ $(CFLAGS) $^ -o $@
%.o: %.cpp %.h
	g++ $(CFLAGS) -c $< -o $@
filelines_gen.o: filelines_gen.cpp find_most_freq.h
	g++ $(CFLAGS) -c $< -o $@
testBlocked.o: testBlocked.cpp find_most_freq.h TestResult.h
filelines_avx.o: filelines_avx.cpp find_most_freq.h
	g++ $(CFLAGS) -c $< -o $@
clean:
	rm -f $(objects) filelines_gen.o testBlocked.o $(targets)