#include "scru128.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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

uint32_t arc4random_mock(void) { return 0x42; }

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

/** Generates increasing IDs even with decreasing or constant timestamp */
void test_decreasing_or_constant_timestamp(void) {
  Scru128Generator g;
  Scru128Id prev, curr;

  uint64_t ts = 0x0123456789ab;
  scru128_initialize_generator(&g);
  assert(scru128_generator_last_status(&g) ==
         SCRU128_GENERATOR_STATUS_NOT_EXECUTED);

  scru128_generate_core(&g, &prev, ts, &arc4random_mock);
  assert(scru128_generator_last_status(&g) ==
         SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP);
  assert(scru128_timestamp(&prev) == ts);

  for (uint64_t i = 0; i < 100000; i++) {
    scru128_generate_core(&g, &curr, ts - (i < 9998 ? i : 9998),
                          &arc4random_mock);
    assert(scru128_generator_last_status(&g) ==
               SCRU128_GENERATOR_STATUS_COUNTER_LO_INC ||
           scru128_generator_last_status(&g) ==
               SCRU128_GENERATOR_STATUS_COUNTER_HI_INC ||
           scru128_generator_last_status(&g) ==
               SCRU128_GENERATOR_STATUS_TIMESTAMP_INC);
    assert(scru128_compare(&prev, &curr) < 0);
    prev = curr;
  }
  assert(scru128_timestamp(&prev) >= ts);
}

/** Breaks increasing order of IDs if timestamp moves backward a lot */
void test_timestamp_rollback(void) {
  Scru128Generator g;
  Scru128Id prev, curr;

  uint64_t ts = 0x0123456789ab;
  scru128_initialize_generator(&g);
  assert(scru128_generator_last_status(&g) ==
         SCRU128_GENERATOR_STATUS_NOT_EXECUTED);

  scru128_generate_core(&g, &prev, ts, &arc4random_mock);
  assert(scru128_generator_last_status(&g) ==
         SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP);
  assert(scru128_timestamp(&prev) == ts);

  scru128_generate_core(&g, &curr, ts - 10000, &arc4random_mock);
  assert(scru128_generator_last_status(&g) ==
         SCRU128_GENERATOR_STATUS_CLOCK_ROLLBACK);
  assert(scru128_compare(&prev, &curr) > 0);
  assert(scru128_timestamp(&curr) == ts - 10000);
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
  run_test(test_decreasing_or_constant_timestamp);
  run_test(test_timestamp_rollback);
  return 0;
}
