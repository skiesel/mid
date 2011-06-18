#include <stdint.h>

typedef struct Rng Rng;
struct Rng {
	uint64_t v;
};

Rng rngnew(uint64_t seed);
uint64_t rngint(Rng r);
double rngdbl(Rng r);
