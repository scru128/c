#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int scru128_get_msec_unixts(uint64_t *out) {
  struct timespec tp;
  int result = clock_gettime(CLOCK_REALTIME, &tp);
  if (result == 0) {
    *out = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  }
  return result;
}

int scru128_get_random_uint32(uint32_t *out) {
  *out = arc4random();
  return 0;
}

#ifdef __cplusplus
} /* extern "C" { */
#endif
