#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "scru128.h"

#define N_SAMPLES 100000
static char samples[N_SAMPLES][32];

void setup() {
  Scru128Generator g;
  scru128_initialize_generator(&g);
  for (int i = 0; i < N_SAMPLES; i++) {
    int err = scru128_generate_string(&g, samples[i]);
    assert(err == 0);
  }
}

/** Generates 26-digit canonical string */
void test_format() {
  for (int i = 0; i < N_SAMPLES; i++) {
    char *e = samples[i];
    assert(strlen(e) == 26);
    assert('0' <= e[0] && e[0] <= '7');
    for (int j = 1; j < 26; j++) {
      assert(('0' <= e[j] && e[j] <= '9') || ('A' <= e[j] && e[j] <= 'V'));
    }
  }
}

/** Generates sortable string representation by creation time */
void test_order() {
  for (int i = 1; i < N_SAMPLES; i++) {
    assert(strcmp(samples[i - 1], samples[i]) < 0);
  }
}

/** Encodes unique sortable pair of timestamp and counter */
void test_timestamp_and_counter() {
  Scru128Id prev, curr;
  scru128_from_str(&prev, samples[0]);
  for (int i = 1; i < N_SAMPLES; i++) {
    scru128_from_str(&curr, samples[i]);
    assert(scru128_timestamp(&prev) < scru128_timestamp(&curr) ||
           (scru128_timestamp(&prev) == scru128_timestamp(&curr) &&
            scru128_counter(&prev) < scru128_counter(&curr)));
    prev = curr;
  }
}

#define run_test(NAME)                                                         \
  do {                                                                         \
    (NAME)();                                                                  \
    printf("  %s: ok\n", #NAME);                                               \
  } while (0)

int main() {
  printf("%s:\n", __FILE__);
  setup();
  run_test(test_format);
  run_test(test_order);
  run_test(test_timestamp_and_counter);
  return 0;
}
