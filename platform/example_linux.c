#include "scru128.h"

#include <sys/random.h>
#include <time.h>

static uint32_t get_random_uint32(void) {
  uint32_t n;
  getentropy(&n, sizeof(uint32_t));
  return n;
}

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
