#include "scru128.h"

#include <stdlib.h> // or <bsd/stdlib.h> on Linux with libbsd
#include <time.h>

int scru128_generate(Scru128Generator *g, uint8_t *id_out) {
  struct timespec tp;
  int err = clock_gettime(CLOCK_REALTIME, &tp);
  if (err) {
    return SCRU128_GENERATOR_STATUS_ERROR;
  }
  uint64_t timestamp = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  return scru128_generate_or_reset_core(g, id_out, timestamp, &arc4random,
                                        10000);
}
