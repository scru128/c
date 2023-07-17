#include "scru128.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define TEXT_BUFFER_SIZE 64

static const uint64_t MAX_UINT48 = ((uint64_t)1 << 48) - 1;
static const uint32_t MAX_UINT24 = ((uint32_t)1 << 24) - 1;
static const uint32_t MAX_UINT32 = UINT32_MAX;

static const int n_generated_strings = 64;
static const char generated_strings[64][TEXT_BUFFER_SIZE] = {
    "036zi0h673abdqnmscasjw0zr", "036zi0h673abdqnmsce4evop9",
    "036zi0h673abdqnmscfh57cfg", "036zi0h673abdqnmsci35e14h",
    "036zi0h673abdqnmsck1yrbz6", "036zi0h673abdqnmscljif809",
    "036zi0h673abdqnmscoaatxny", "036zi0h673abdqnmscp02lhx5",
    "036zi0h673abdqnmscqm37inm", "036zi0h673abdqnmscswr3ty1",
    "036zi0h673abdqnmscuv3uszm", "036zi0h673abdqnmscwsc9917",
    "036zi0h673abdqnmsczws9jxx", "036zi0h678r6gib1nu7o9to20",
    "036zi0h678r6gib1nu826pvxq", "036zi0h678r6gib1nua8aowo0",
    "036zi0h678r6gib1nuc0dlsrk", "036zi0h678r6gib1nufjup6ni",
    "036zi0h678r6gib1nug0ebtqp", "036zi0h678r6gib1nuib9h8z9",
    "036zi0h678r6gib1nuk2kai61", "036zi0h678r6gib1nunr24dib",
    "036zi0h678r6gib1nupomdw59", "036zi0h678r6gib1nuqpth1lm",
    "036zi0h678r6gib1nutedyyw3", "036zi0h678r6gib1nuunymfl3",
    "036zi0h678r6gib1nuwcpdbf4", "036zi0h678r6gib1nuxz94tmh",
    "036zi0h678r6gib1nuzrdq2tn", "036zi0h678r6gib1nv2ocjn08",
    "036zi0h678r6gib1nv52wltuq", "036zi0h678r6gib1nv5y34yho",
    "036zi0h678r6gib1nv8ddlio7", "036zi0h678r6gib1nv9iht0cz",
    "036zi0h678r6gib1nvbpnxenu", "036zi0h678r6gib1nvf2bgvel",
    "036zi0h678r6gib1nvgdp6zxu", "036zi0h678r6gib1nvihj5lv9",
    "036zi0h678r6gib1nvk6h9atk", "036zi0h678r6gib1nvmq9cr3y",
    "036zi0h678r6gib1nvnj0xhqu", "036zi0h678r6gib1nvqk67rdy",
    "036zi0h678r6gib1nvrl1yeaa", "036zi0h678r6gib1nvtwz18qe",
    "036zi0h678r6gib1nvv7tiyos", "036zi0h678r6gib1nvxcncbvt",
    "036zi0h678r6gib1nw0rx7126", "036zi0h678r6gib1nw2nbc6y6",
    "036zi0h678r6gib1nw34fdlv3", "036zi0h678r6gib1nw5raflpg",
    "036zi0h678r6gib1nw7m3ombt", "036zi0h678r6gib1nwa06bdym",
    "036zi0h678r6gib1nwbaoge5x", "036zi0h678r6gib1nweckniqz",
    "036zi0h678r6gib1nwf3tosni", "036zi0h678r6gib1nwhsdqgj5",
    "036zi0h678r6gib1nwkiwq70l", "036zi0h678r6gib1nwmgx8x8u",
    "036zi0h678r6gib1nwnbb89pg", "036zi0h678r6gib1nwqh7so5b",
    "036zi0h678r6gib1nwrrh9ro8", "036zi0h678r6gib1nwtx3drds",
    "036zi0h678r6gib1nwwkxidvm", "036zi0h678r6gib1nwx9nxgkj",
};

/** Encodes and decodes prepared cases correctly */
void test_encode_decode(void) {
  struct Case {
    uint64_t timestamp;
    uint32_t counter_hi;
    uint32_t counter_lo;
    uint32_t entropy;
    char text[TEXT_BUFFER_SIZE];
  };

  struct Case cases[] = {
      {0, 0, 0, 0, "0000000000000000000000000"},
      {MAX_UINT48, 0, 0, 0, "F5LXX1ZZ5K6TP71GEEH2DB7K0"},
      {MAX_UINT48, 0, 0, 0, "f5lxx1zz5k6tp71geeh2db7k0"},
      {0, MAX_UINT24, 0, 0, "0000000005GV2R2KJWR7N8XS0"},
      {0, MAX_UINT24, 0, 0, "0000000005gv2r2kjwr7n8xs0"},
      {0, 0, MAX_UINT24, 0, "00000000000000JPIA7QL4HS0"},
      {0, 0, MAX_UINT24, 0, "00000000000000jpia7ql4hs0"},
      {0, 0, 0, MAX_UINT32, "0000000000000000001Z141Z3"},
      {0, 0, 0, MAX_UINT32, "0000000000000000001z141z3"},
      {MAX_UINT48, MAX_UINT24, MAX_UINT24, MAX_UINT32,
       "F5LXX1ZZ5PNORYNQGLHZMSP33"},
      {MAX_UINT48, MAX_UINT24, MAX_UINT24, MAX_UINT32,
       "f5lxx1zz5pnorynqglhzmsp33"},
  };
  const int n_cases = sizeof(cases) / sizeof(cases[0]);

  for (int i = 0; i < n_cases; i++) {
    struct Case *e = &cases[i];
    uint8_t from_fields[SCRU128_LEN], from_string[SCRU128_LEN];
    int err;
    err = scru128_from_fields(from_fields, e->timestamp, e->counter_hi,
                              e->counter_lo, e->entropy);
    assert(err == 0);
    err = scru128_from_str(from_string, e->text);
    assert(err == 0);

    size_t text_len = strlen(e->text);
    char text_buffer[TEXT_BUFFER_SIZE];

    assert(scru128_timestamp(from_fields) == e->timestamp);
    assert(scru128_counter_hi(from_fields) == e->counter_hi);
    assert(scru128_counter_lo(from_fields) == e->counter_lo);
    assert(scru128_entropy(from_fields) == e->entropy);
    scru128_to_str(from_fields, text_buffer);
    assert(strlen(text_buffer) == text_len);
    for (size_t j = 0; j < text_len; j++) {
      assert(text_buffer[j] == tolower(e->text[j]));
    }

    assert(scru128_timestamp(from_string) == e->timestamp);
    assert(scru128_counter_hi(from_string) == e->counter_hi);
    assert(scru128_counter_lo(from_string) == e->counter_lo);
    assert(scru128_entropy(from_string) == e->entropy);
    scru128_to_str(from_string, text_buffer);
    assert(strlen(text_buffer) == text_len);
    for (size_t j = 0; j < text_len; j++) {
      assert(text_buffer[j] == tolower(e->text[j]));
    }
  }
}

/** Raises error if an invalid string representation is supplied */
void test_string_validation(void) {
  char cases[][TEXT_BUFFER_SIZE] = {
      "",
      " 036z8puq4tsxsigk6o19y164q",
      "036z8puq54qny1vq3hcbrkweb ",
      " 036z8puq54qny1vq3helivwax ",
      "+036z8puq54qny1vq3hfcv3ss0",
      "-036z8puq54qny1vq3hhy8u1ch",
      "+36z8puq54qny1vq3hjq48d9p",
      "-36z8puq5a7j0ti08oz6zdrdy",
      "036z8puq5a7j0t_08p2cdz28v",
      "036z8pu-5a7j0ti08p3ol8ool",
      "036z8puq5a7j0ti08p4j 6cya",
      "f5lxx1zz5pnorynqglhzmsp34",
      "zzzzzzzzzzzzzzzzzzzzzzzzz",
  };
  const int n_cases = sizeof(cases) / sizeof(cases[0]);

  for (int i = 0; i < n_cases; i++) {
    uint8_t x[SCRU128_LEN];
    int err = scru128_from_str(x, cases[i]);
    assert(err != 0);
  }
}

/** Has symmetric converters from/to various values */
void test_symmetric_converters(void) {
  int n_cases = 0;
  uint8_t cases[70][SCRU128_LEN];
  scru128_from_fields(cases[n_cases++], 0, 0, 0, 0);
  scru128_from_fields(cases[n_cases++], MAX_UINT48, 0, 0, 0);
  scru128_from_fields(cases[n_cases++], 0, MAX_UINT24, 0, 0);
  scru128_from_fields(cases[n_cases++], 0, 0, MAX_UINT24, 0);
  scru128_from_fields(cases[n_cases++], 0, 0, 0, MAX_UINT32);
  scru128_from_fields(cases[n_cases++], MAX_UINT48, MAX_UINT24, MAX_UINT24,
                      MAX_UINT32);

  for (int i = 0; i < n_generated_strings; i++) {
    scru128_from_str(cases[n_cases++], generated_strings[i]);
  }

  for (int i = 0; i < n_cases; i++) {
    uint8_t *e = cases[i];
    uint8_t id_buffer[SCRU128_LEN];
    int err;

    scru128_copy(id_buffer, e);
    assert(scru128_compare(id_buffer, e) == 0);
    assert(memcmp(id_buffer, e, SCRU128_LEN) == 0);
    char text_buffer[TEXT_BUFFER_SIZE];
    scru128_to_str(e, text_buffer);
    err = scru128_from_str(id_buffer, text_buffer);
    assert(err == 0);
    assert(scru128_compare(id_buffer, e) == 0);
    assert(memcmp(id_buffer, e, SCRU128_LEN) == 0);
    err = scru128_from_fields(id_buffer, scru128_timestamp(e),
                              scru128_counter_hi(e), scru128_counter_lo(e),
                              scru128_entropy(e));
    assert(err == 0);
    assert(scru128_compare(id_buffer, e) == 0);
    assert(memcmp(id_buffer, e, SCRU128_LEN) == 0);
  }
}

/** Supports comparison methods */
void test_comparison_methods(void) {
  int n_cases = 0;
  uint8_t ordered[73][SCRU128_LEN];
  scru128_from_fields(ordered[n_cases++], 0, 0, 0, 0);
  scru128_from_fields(ordered[n_cases++], 0, 0, 0, 1);
  scru128_from_fields(ordered[n_cases++], 0, 0, 0, MAX_UINT32);
  scru128_from_fields(ordered[n_cases++], 0, 0, 1, 0);
  scru128_from_fields(ordered[n_cases++], 0, 0, MAX_UINT24, 0);
  scru128_from_fields(ordered[n_cases++], 0, 1, 0, 0);
  scru128_from_fields(ordered[n_cases++], 0, MAX_UINT24, 0, 0);
  scru128_from_fields(ordered[n_cases++], 1, 0, 0, 0);
  scru128_from_fields(ordered[n_cases++], 2, 0, 0, 0);

  for (int i = 0; i < n_generated_strings; i++) {
    scru128_from_str(ordered[n_cases++], generated_strings[i]);
  }

  uint8_t *prev = ordered[0];
  for (int i = 1; i < n_cases; i++) {
    uint8_t *curr = ordered[i];
    assert(scru128_compare(curr, prev) > 0);
    assert(memcmp(curr, prev, SCRU128_LEN) > 0);
    assert(scru128_compare(prev, curr) < 0);
    assert(memcmp(prev, curr, SCRU128_LEN) < 0);

    uint8_t clone[SCRU128_LEN];
    scru128_copy(clone, curr);
    assert(curr != clone);
    assert(scru128_compare(curr, clone) == 0);
    assert(memcmp(curr, clone, SCRU128_LEN) == 0);

    prev = curr;
  }
}

uint32_t arc4random_mock(void) { return 0x42; }

/** Generates increasing IDs even with decreasing or constant timestamp */
void test_decreasing_or_constant_timestamp_reset(void) {
  Scru128Generator g;
  uint8_t prev[SCRU128_LEN], curr[SCRU128_LEN];

  uint64_t ts = 0x0123456789ab;
  scru128_generator_init(&g);
  int status =
      scru128_generate_or_reset_core(&g, prev, ts, &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP);
  assert(scru128_timestamp(prev) == ts);

  for (uint64_t i = 0; i < 100000; i++) {
    status = scru128_generate_or_reset_core(
        &g, curr, ts - (i < 9999 ? i : 9999), &arc4random_mock, 10000);
    assert(status == SCRU128_GENERATOR_STATUS_COUNTER_LO_INC ||
           status == SCRU128_GENERATOR_STATUS_COUNTER_HI_INC ||
           status == SCRU128_GENERATOR_STATUS_TIMESTAMP_INC);
    assert(scru128_compare(prev, curr) < 0);
    assert(memcmp(prev, curr, SCRU128_LEN) < 0);
    memcpy(prev, curr, SCRU128_LEN);
  }
  assert(scru128_timestamp(prev) >= ts);
}

/** Breaks increasing order of IDs if timestamp goes backwards a lot */
void test_timestamp_rollback_reset(void) {
  Scru128Generator g;
  uint8_t prev[SCRU128_LEN], curr[SCRU128_LEN];

  uint64_t ts = 0x0123456789ab;
  scru128_generator_init(&g);
  int status =
      scru128_generate_or_reset_core(&g, prev, ts, &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP);
  assert(scru128_timestamp(prev) == ts);

  status = scru128_generate_or_reset_core(&g, curr, ts - 10000,
                                          &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_COUNTER_LO_INC ||
         status == SCRU128_GENERATOR_STATUS_COUNTER_HI_INC ||
         status == SCRU128_GENERATOR_STATUS_TIMESTAMP_INC);
  assert(scru128_compare(prev, curr) < 0);
  assert(memcmp(prev, curr, SCRU128_LEN) < 0);

  memcpy(prev, curr, SCRU128_LEN);
  status = scru128_generate_or_reset_core(&g, curr, ts - 10001,
                                          &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_ROLLBACK_RESET);
  assert(scru128_compare(prev, curr) > 0);
  assert(memcmp(prev, curr, SCRU128_LEN) > 0);
  assert(scru128_timestamp(curr) == ts - 10001);

  memcpy(prev, curr, SCRU128_LEN);
  status = scru128_generate_or_reset_core(&g, curr, ts - 10002,
                                          &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_COUNTER_LO_INC ||
         status == SCRU128_GENERATOR_STATUS_COUNTER_HI_INC ||
         status == SCRU128_GENERATOR_STATUS_TIMESTAMP_INC);
  assert(scru128_compare(prev, curr) < 0);
  assert(memcmp(prev, curr, SCRU128_LEN) < 0);
}

/** Generates increasing IDs even with decreasing or constant timestamp */
void test_decreasing_or_constant_timestamp_abort(void) {
  Scru128Generator g;
  uint8_t prev[SCRU128_LEN], curr[SCRU128_LEN];

  uint64_t ts = 0x0123456789ab;
  scru128_generator_init(&g);
  int status =
      scru128_generate_or_abort_core(&g, prev, ts, &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP);
  assert(scru128_timestamp(prev) == ts);

  for (uint64_t i = 0; i < 100000; i++) {
    status = scru128_generate_or_abort_core(
        &g, curr, ts - (i < 9999 ? i : 9999), &arc4random_mock, 10000);
    assert(status == SCRU128_GENERATOR_STATUS_COUNTER_LO_INC ||
           status == SCRU128_GENERATOR_STATUS_COUNTER_HI_INC ||
           status == SCRU128_GENERATOR_STATUS_TIMESTAMP_INC);
    assert(scru128_compare(prev, curr) < 0);
    assert(memcmp(prev, curr, SCRU128_LEN) < 0);
    memcpy(prev, curr, SCRU128_LEN);
  }
  assert(scru128_timestamp(prev) >= ts);
}

/** Returns error if timestamp goes backwards a lot */
void test_timestamp_rollback_abort(void) {
  Scru128Generator g;
  uint8_t prev[SCRU128_LEN], curr[SCRU128_LEN];

  uint64_t ts = 0x0123456789ab;
  scru128_generator_init(&g);
  int status =
      scru128_generate_or_abort_core(&g, prev, ts, &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP);
  assert(scru128_timestamp(prev) == ts);

  status = scru128_generate_or_abort_core(&g, curr, ts - 10000,
                                          &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_COUNTER_LO_INC ||
         status == SCRU128_GENERATOR_STATUS_COUNTER_HI_INC ||
         status == SCRU128_GENERATOR_STATUS_TIMESTAMP_INC);
  assert(scru128_compare(prev, curr) < 0);
  assert(memcmp(prev, curr, SCRU128_LEN) < 0);

  memcpy(curr, prev, SCRU128_LEN);
  status = scru128_generate_or_abort_core(&g, curr, ts - 10001,
                                          &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_ROLLBACK_ABORT);
  assert(scru128_compare(prev, curr) == 0);
  assert(memcmp(prev, curr, SCRU128_LEN) == 0); // untouched

  status = scru128_generate_or_abort_core(&g, curr, ts - 10002,
                                          &arc4random_mock, 10000);
  assert(status == SCRU128_GENERATOR_STATUS_ROLLBACK_ABORT);
  assert(scru128_compare(prev, curr) == 0);
  assert(memcmp(prev, curr, SCRU128_LEN) == 0); // untouched
}

#define run_test(NAME)                                                         \
  do {                                                                         \
    (NAME)();                                                                  \
    printf("  %s: ok\n", #NAME);                                               \
  } while (0)

int main(void) {
  printf("%s:\n", __FILE__);
  run_test(test_encode_decode);
  run_test(test_string_validation);
  run_test(test_symmetric_converters);
  run_test(test_comparison_methods);
  run_test(test_decreasing_or_constant_timestamp_reset);
  run_test(test_timestamp_rollback_reset);
  run_test(test_decreasing_or_constant_timestamp_abort);
  run_test(test_timestamp_rollback_abort);
  return 0;
}
