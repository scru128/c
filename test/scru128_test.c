#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "scru128.h"

#define N_SAMPLES 100000
static char samples[N_SAMPLES][32];

void setup(void) {
  Scru128Generator g;
  scru128_initialize_generator(&g);
  for (int i = 0; i < N_SAMPLES; i++) {
    int err = scru128_generate_string(&g, samples[i]);
    assert(err == 0);
  }
}

/** Generates 25-digit canonical string */
void test_format(void) {
  for (int i = 0; i < N_SAMPLES; i++) {
    char *e = samples[i];
    assert(strlen(e) == 25);
    for (int j = 0; j < 25; j++) {
      assert(('0' <= e[j] && e[j] <= '9') || ('A' <= e[j] && e[j] <= 'Z'));
    }
  }
}

/** Generates sortable string representation by creation time */
void test_order(void) {
  for (int i = 1; i < N_SAMPLES; i++) {
    assert(strcmp(samples[i - 1], samples[i]) < 0);
  }
}

/** Encodes unique sortable tuple of timestamp and counters */
void test_timestamp_and_counters(void) {
  Scru128Id prev, curr;
  scru128_from_str(&prev, samples[0]);
  for (int i = 1; i < N_SAMPLES; i++) {
    scru128_from_str(&curr, samples[i]);
    assert(scru128_timestamp(&prev) < scru128_timestamp(&curr) ||
           (scru128_timestamp(&prev) == scru128_timestamp(&curr) &&
            scru128_counter_hi(&prev) < scru128_counter_hi(&curr)) ||
           (scru128_timestamp(&prev) == scru128_timestamp(&curr) &&
            scru128_counter_hi(&prev) == scru128_counter_hi(&curr) &&
            scru128_counter_lo(&prev) < scru128_counter_lo(&curr)));
    prev = curr;
  }
}

#define run_test(NAME)                                                         \
  do {                                                                         \
    (NAME)();                                                                  \
    printf("  %s: ok\n", #NAME);                                               \
  } while (0)

int main(void) {
  printf("%s:\n", __FILE__);
  setup();
  run_test(test_format);
  run_test(test_order);
  run_test(test_timestamp_and_counters);
  return 0;
}
