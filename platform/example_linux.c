#include "scru128.h"

#include <sys/random.h>
#include <time.h>

static uint32_t get_random_uint32(void) {
  uint32_t n;
  getrandom(&n, sizeof(n), 0);
  return n;
}

/** @warning This example is NOT thread-safe. */
int scru128_generate(Scru128Generator *g, Scru128Id *out) {
  struct timespec tp;
  int err = clock_gettime(CLOCK_REALTIME, &tp);
  if (err) {
    return SCRU128_GENERATOR_STATUS_ERROR;
  }
  uint64_t timestamp = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  return scru128_generate_core(g, out, timestamp, &get_random_uint32);
}
