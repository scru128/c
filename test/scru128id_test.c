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
    "036ZI0H673ABDQNMSCASJW0ZR", "036ZI0H673ABDQNMSCE4EVOP9",
    "036ZI0H673ABDQNMSCFH57CFG", "036ZI0H673ABDQNMSCI35E14H",
    "036ZI0H673ABDQNMSCK1YRBZ6", "036ZI0H673ABDQNMSCLJIF809",
    "036ZI0H673ABDQNMSCOAATXNY", "036ZI0H673ABDQNMSCP02LHX5",
    "036ZI0H673ABDQNMSCQM37INM", "036ZI0H673ABDQNMSCSWR3TY1",
    "036ZI0H673ABDQNMSCUV3USZM", "036ZI0H673ABDQNMSCWSC9917",
    "036ZI0H673ABDQNMSCZWS9JXX", "036ZI0H678R6GIB1NU7O9TO20",
    "036ZI0H678R6GIB1NU826PVXQ", "036ZI0H678R6GIB1NUA8AOWO0",
    "036ZI0H678R6GIB1NUC0DLSRK", "036ZI0H678R6GIB1NUFJUP6NI",
    "036ZI0H678R6GIB1NUG0EBTQP", "036ZI0H678R6GIB1NUIB9H8Z9",
    "036ZI0H678R6GIB1NUK2KAI61", "036ZI0H678R6GIB1NUNR24DIB",
    "036ZI0H678R6GIB1NUPOMDW59", "036ZI0H678R6GIB1NUQPTH1LM",
    "036ZI0H678R6GIB1NUTEDYYW3", "036ZI0H678R6GIB1NUUNYMFL3",
    "036ZI0H678R6GIB1NUWCPDBF4", "036ZI0H678R6GIB1NUXZ94TMH",
    "036ZI0H678R6GIB1NUZRDQ2TN", "036ZI0H678R6GIB1NV2OCJN08",
    "036ZI0H678R6GIB1NV52WLTUQ", "036ZI0H678R6GIB1NV5Y34YHO",
    "036ZI0H678R6GIB1NV8DDLIO7", "036ZI0H678R6GIB1NV9IHT0CZ",
    "036ZI0H678R6GIB1NVBPNXENU", "036ZI0H678R6GIB1NVF2BGVEL",
    "036ZI0H678R6GIB1NVGDP6ZXU", "036ZI0H678R6GIB1NVIHJ5LV9",
    "036ZI0H678R6GIB1NVK6H9ATK", "036ZI0H678R6GIB1NVMQ9CR3Y",
    "036ZI0H678R6GIB1NVNJ0XHQU", "036ZI0H678R6GIB1NVQK67RDY",
    "036ZI0H678R6GIB1NVRL1YEAA", "036ZI0H678R6GIB1NVTWZ18QE",
    "036ZI0H678R6GIB1NVV7TIYOS", "036ZI0H678R6GIB1NVXCNCBVT",
    "036ZI0H678R6GIB1NW0RX7126", "036ZI0H678R6GIB1NW2NBC6Y6",
    "036ZI0H678R6GIB1NW34FDLV3", "036ZI0H678R6GIB1NW5RAFLPG",
    "036ZI0H678R6GIB1NW7M3OMBT", "036ZI0H678R6GIB1NWA06BDYM",
    "036ZI0H678R6GIB1NWBAOGE5X", "036ZI0H678R6GIB1NWECKNIQZ",
    "036ZI0H678R6GIB1NWF3TOSNI", "036ZI0H678R6GIB1NWHSDQGJ5",
    "036ZI0H678R6GIB1NWKIWQ70L", "036ZI0H678R6GIB1NWMGX8X8U",
    "036ZI0H678R6GIB1NWNBB89PG", "036ZI0H678R6GIB1NWQH7SO5B",
    "036ZI0H678R6GIB1NWRRH9RO8", "036ZI0H678R6GIB1NWTX3DRDS",
    "036ZI0H678R6GIB1NWWKXIDVM", "036ZI0H678R6GIB1NWX9NXGKJ",
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
    Scru128Id from_fields, from_string;
    int err;
    err = scru128_from_fields(&from_fields, e->timestamp, e->counter_hi,
                              e->counter_lo, e->entropy);
    assert(err == 0);
    err = scru128_from_str(&from_string, e->text);
    assert(err == 0);

    size_t text_len = strlen(e->text);
    char text_buffer[TEXT_BUFFER_SIZE];

    assert(scru128_timestamp(&from_fields) == e->timestamp);
    assert(scru128_counter_hi(&from_fields) == e->counter_hi);
    assert(scru128_counter_lo(&from_fields) == e->counter_lo);
    assert(scru128_entropy(&from_fields) == e->entropy);
    scru128_to_str(&from_fields, text_buffer);
    assert(strlen(text_buffer) == text_len);
    for (size_t j = 0; j < text_len; j++) {
      assert(text_buffer[j] == toupper(e->text[j]));
    }

    assert(scru128_timestamp(&from_string) == e->timestamp);
    assert(scru128_counter_hi(&from_string) == e->counter_hi);
    assert(scru128_counter_lo(&from_string) == e->counter_lo);
    assert(scru128_entropy(&from_string) == e->entropy);
    scru128_to_str(&from_string, text_buffer);
    assert(strlen(text_buffer) == text_len);
    for (size_t j = 0; j < text_len; j++) {
      assert(text_buffer[j] == toupper(e->text[j]));
    }
  }
}

/** Raises error if an invalid string representation is supplied */
void test_string_validation(void) {
  char cases[][TEXT_BUFFER_SIZE] = {
      "",
      " 036Z8PUQ4TSXSIGK6O19Y164Q",
      "036Z8PUQ54QNY1VQ3HCBRKWEB ",
      " 036Z8PUQ54QNY1VQ3HELIVWAX ",
      "+036Z8PUQ54QNY1VQ3HFCV3SS0",
      "-036Z8PUQ54QNY1VQ3HHY8U1CH",
      "+36Z8PUQ54QNY1VQ3HJQ48D9P",
      "-36Z8PUQ5A7J0TI08OZ6ZDRDY",
      "036Z8PUQ5A7J0T_08P2CDZ28V",
      "036Z8PU-5A7J0TI08P3OL8OOL",
      "036Z8PUQ5A7J0TI08P4J 6CYA",
      "F5LXX1ZZ5PNORYNQGLHZMSP34",
      "ZZZZZZZZZZZZZZZZZZZZZZZZZ",
  };
  const int n_cases = sizeof(cases) / sizeof(cases[0]);

  for (int i = 0; i < n_cases; i++) {
    Scru128Id x;
    int err = scru128_from_str(&x, cases[i]);
    assert(err != 0);
  }
}

/** Has symmetric converters from/to various values */
void test_symmetric_converters(void) {
  int n_cases = 0;
  Scru128Id cases[70];
  scru128_from_fields(&cases[n_cases++], 0, 0, 0, 0);
  scru128_from_fields(&cases[n_cases++], MAX_UINT48, 0, 0, 0);
  scru128_from_fields(&cases[n_cases++], 0, MAX_UINT24, 0, 0);
  scru128_from_fields(&cases[n_cases++], 0, 0, MAX_UINT24, 0);
  scru128_from_fields(&cases[n_cases++], 0, 0, 0, MAX_UINT32);
  scru128_from_fields(&cases[n_cases++], MAX_UINT48, MAX_UINT24, MAX_UINT24,
                      MAX_UINT32);

  for (int i = 0; i < n_generated_strings; i++) {
    scru128_from_str(&cases[n_cases++], generated_strings[i]);
  }

  for (int i = 0; i < n_cases; i++) {
    Scru128Id *e = &cases[i];
    Scru128Id id_buffer;
    int err;

    uint8_t bytes_buffer[16];
    scru128_to_bytes(e, bytes_buffer);
    err = scru128_from_bytes(&id_buffer, bytes_buffer);
    assert(err == 0);
    assert(scru128_compare(&id_buffer, e) == 0);
    char text_buffer[TEXT_BUFFER_SIZE];
    scru128_to_str(e, text_buffer);
    err = scru128_from_str(&id_buffer, text_buffer);
    assert(err == 0);
    assert(scru128_compare(&id_buffer, e) == 0);
    err = scru128_from_fields(&id_buffer, scru128_timestamp(e),
                              scru128_counter_hi(e), scru128_counter_lo(e),
                              scru128_entropy(e));
    assert(err == 0);
    assert(scru128_compare(&id_buffer, e) == 0);
  }
}

/** Supports comparison methods */
void test_comparison_methods(void) {
  int n_cases = 0;
  Scru128Id ordered[73];
  scru128_from_fields(&ordered[n_cases++], 0, 0, 0, 0);
  scru128_from_fields(&ordered[n_cases++], 0, 0, 0, 1);
  scru128_from_fields(&ordered[n_cases++], 0, 0, 0, MAX_UINT32);
  scru128_from_fields(&ordered[n_cases++], 0, 0, 1, 0);
  scru128_from_fields(&ordered[n_cases++], 0, 0, MAX_UINT24, 0);
  scru128_from_fields(&ordered[n_cases++], 0, 1, 0, 0);
  scru128_from_fields(&ordered[n_cases++], 0, MAX_UINT24, 0, 0);
  scru128_from_fields(&ordered[n_cases++], 1, 0, 0, 0);
  scru128_from_fields(&ordered[n_cases++], 2, 0, 0, 0);

  for (int i = 0; i < n_generated_strings; i++) {
    scru128_from_str(&ordered[n_cases++], generated_strings[i]);
  }

  Scru128Id *prev = &ordered[0];
  for (int i = 1; i < n_cases; i++) {
    Scru128Id *curr = &ordered[i];
    assert(scru128_compare(curr, prev) > 0);
    assert(scru128_compare(prev, curr) < 0);

    Scru128Id clone = *curr;
    assert(curr != &clone);
    assert(scru128_compare(curr, &clone) == 0);

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
  run_test(test_encode_decode);
  run_test(test_string_validation);
  run_test(test_symmetric_converters);
  run_test(test_comparison_methods);
  return 0;
}
