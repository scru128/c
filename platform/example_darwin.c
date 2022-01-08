#ifdef SCRU128_WITH_GENERATOR

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

#endif /* #ifdef SCRU128_WITH_GENERATOR */

#ifdef SCRU128_WITH_LOGGING

#include <stdio.h>

void scru128_log_warn(const char *message) {
  fprintf(stderr, "WARN: scru128: %s\n", message);
}

void scru128_log_info(const char *message) {
  fprintf(stderr, "INFO: scru128: %s\n", message);
}

#endif /* #ifdef SCRU128_WITH_LOGGING */
