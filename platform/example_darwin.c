#include <stdint.h>
#include <stdlib.h>
#include <time.h>

int scru128_get_msec_unixts(uint64_t *out) {
  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  *out = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  return 0;
}

int scru128_get_random_uint32(uint32_t *out) {
  *out = arc4random();
  return 0;
}
