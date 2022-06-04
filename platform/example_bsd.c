#include "scru128.h"

#include <stdlib.h> // or <bsd/stdlib.h> on Linux with libbsd
#include <time.h>

/** @warning This example is NOT thread-safe. */
int scru128_generate(Scru128Generator *g, Scru128Id *out) {
  struct timespec tp;
  int err = clock_gettime(CLOCK_REALTIME, &tp);
  if (err) {
    scru128_generator_report_error(g);
    return err;
  }
  uint64_t timestamp = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  return scru128_generate_core(g, out, timestamp, &arc4random);
}
