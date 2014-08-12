//
// Random reads
//
// Nick Zaborovsky
// Aug-2014
//
// + Ulrich! + BLepers and FGaud


#include <cstdio>
#include <vector>
#include <random>
#include <algorithm>
#include "stopwatch.h"


#define RANDOM_RANGE
#define NPAD_POWER	3
#define NPAD ((1<<NPAD_POWER) - 1)

static FILE *h_log;

struct chunk_t {
	chunk_t *p;
#if (NPAD!=0)
	long int payload[NPAD];
#endif
};


volatile unsigned no_optimize = 0;
#define NO_OPTIMIZE(x)	no_optimize += unsigned(x);


void workCycle(const unsigned logSize, const unsigned logRangeElements) {
	chunk_t *const mem = new chunk_t[1 << logSize];

	// Creates circle list
	chunk_t *first = mem;
	chunk_t *cur = first;

	srand(time(0));

	// Set elements pointers to random elements from the list
	for (int i = 0; i < (1 << logSize); i += 1) {
#ifdef RANDOM_RANGE
		//std::vector<int> indices((1 << logRangeElements) - 1);
		//for (unsigned d = 0; d < indices.size(); ++d) {
		//	indices[d] = d;
		//}
		//std::shuffle(indices.begin(), indices.end(), std::default_random_engine(time(0)));
		unsigned rand_range = i / (1 << logRangeElements);
		// Select the range within there will be random accesses
		cur->p = first + ((1 << logRangeElements) * rand_range) + rand() % (1 << logRangeElements);
#else // RANDOM_RANGE
		cur->p = first + rand() % (1 << logSize);
#endif // RANDOM_RANGE
		cur += 1;
	}

	register int n, m;
	swatch t;
	t.reset();
	for (n = 0; n < 128; ++n) {
		for (m = 0; m < (1 << logSize); ++m) {
			NO_OPTIMIZE((first + m)->p->p);
		}
	}
	long long elapsed = t.get();
	fprintf(h_log, "%d\t%d\t%d\t%d\n", logRangeElements,
		sizeof(chunk_t) * (1 << logSize) / (1 << 10), 1 << logSize, 1000 * elapsed / (1 << logSize));
	printf("RANGE: 2^%d, WS: %d KB, items: %d, time/128items: %d nanosec\n", logRangeElements,
		sizeof(chunk_t) * (1 << logSize) / (1 << 10), 1 << logSize, 1000 * elapsed / (1 << logSize));

	delete[] mem;
}


int main() {
	if (NULL == (h_log = fopen("log.txt", "a+"))) {
		printf("Cannot open the log file...\n");
		getchar();
		return 0;
	}
	
	printf("--- Chunk size: %u ---\n", sizeof(chunk_t));
	fprintf(h_log, "--- Chunk size: %u ---\n", sizeof(chunk_t));
	fprintf(h_log, "RANGE: power,\t WS: KB,\t items num,\t time/128items: picosec\n");
#ifndef RANDOM_RANGE
	for (unsigned i = 11 - NPAD_POWER; i < 29 - NPAD_POWER; ++i) {
		workCycle(i, 0); // Second param is not used - stub
	}
#else
	for (unsigned i = 3; i < 29 - NPAD_POWER; ++i) {
		workCycle(26 - NPAD_POWER, i);
	}
#endif
	fclose(h_log);
	getchar();
	return 1;
}