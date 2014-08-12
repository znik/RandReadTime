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


#define NPAD_POWER	4
#define NPAD ((1<<NPAD_POWER) - 1)
#define TWICE(x)	x x
#define TIMES_256(x)	TWICE(TWICE(TWICE(TWICE(TWICE(TWICE(TWICE(TWICE(x))))))))


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
	for (int i = 0; i < (1 << (logSize - logRangeElements)); ++i) {
		std::vector<int> indices((1 << logRangeElements) - 1);
		for (unsigned d = 0; d < indices.size(); ++d) {
			indices[d] = d + 1;
		}

		std::shuffle(indices.begin(), indices.end(), std::default_random_engine(time(0)));

		if (indices.size() > 0 && 0 == indices[0])
			std::swap(indices[0], indices[1]);

		for (int b = 0; b < indices.size(); ++b) {
			cur->p = first + ((1 << logRangeElements) * i) + indices[b];
			cur = cur->p;
		}
		cur->p = first + ((1 << logRangeElements) * (i + 1));
		cur = cur->p;
	}

	register int n, m;
	register chunk_t* x = first->p;

	swatch t;
	t.reset();
	for (n = 0; n < 128; ++n) {
		register int remain = (1 << logSize) / 256;
		// Increase load instructions ratio in respect to branch instructions.
		x = first->p;
		while(remain--) {
			TIMES_256(x = x->p;)
		}
	}
	long long elapsed = t.get();
	fprintf(h_log, "%d\t%d\t%d\t%d\n", logRangeElements,
		sizeof(chunk_t) * (1 << logSize) / (1 << 10), 1 << logSize, 1000 * elapsed / (1 << logSize));
	printf("RANGE: 2^%d, WS: %d KB, items: %d, time/128items: %d nanosec\n", logRangeElements - 1,
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
	for (unsigned i = 0; i < 27 - NPAD_POWER; ++i) {
		workCycle(26 - NPAD_POWER, i);
	}
	fclose(h_log);
	getchar();
	return 1;
}