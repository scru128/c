#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "scru128.h"

#define TEXT_BUFFER_SIZE 64

static const uint64_t MAX_UINT44 = ((uint64_t)1 << 44) - 1;
static const uint32_t MAX_UINT28 = ((uint32_t)1 << 28) - 1;
static const uint32_t MAX_UINT24 = ((uint32_t)1 << 24) - 1;
static const uint32_t MAX_UINT32 = UINT32_MAX;

static const int n_generated_strings = 64;
static const char generated_strings[64][TEXT_BUFFER_SIZE] = {
    "00TGAMS8N0BSNJ91RTSBCLB5IB", "00TGAMS8N0BSNJB1RTSBFCNJMH",
    "00TGAMS8N0BSNJD1RTS8IQ0QAT", "00TGAMS8N0BSNJF1RTSABBK78K",
    "00TGAMS8N0BSNJH1RTSA77JQ44", "00TGAMS8N0BSNJJ1RTS8N83SDM",
    "00TGAMS8N0BSNJL1RTSAEEH1MN", "00TGAMS8N0BSNJN1RTS80VLCKA",
    "00TGAMS8N0BSNJP1RTS9GG1HB9", "00TGAMS8N0BSNJR1RTSAH77CPO",
    "00TGAMS8N0BSNJT1RTS9J3OJM9", "00TGAMS8N0BSNJV1RTSBSMP1BO",
    "00TGAMS8N0BSNK11RTSBHUJ4JG", "00TGAMS8N0BSNK31RTS8NR8AMD",
    "00TGAMS8N0BSNK51RTS8PPDVER", "00TGAMS8N0BSNK71RTS89GIKHQ",
    "00TGAMSTIRMI5OS4J54E6VBE30", "00TGAMSTIRMI5OU4J54E01RQ1G",
    "00TGAMSTIRMI5P04J54CMQ4QS8", "00TGAMSTIRMI5P24J54EM4DJMU",
    "00TGAMSTIRMI5P44J54DQ3AMDE", "00TGAMSTIRMI5P64J54D1B16JE",
    "00TGAMSTIRMI5P84J54CI89IKO", "00TGAMSTIRMI5PA4J54C0P49GO",
    "00TGAMSTIRMI5PC4J54C5RGQ0S", "00TGAMSTIRMI5PE4J54C5GJ0SC",
    "00TGAMSTIRMI5PG4J54C49NFJD", "00TGAMSTIRMI5PI4J54CCE6TVM",
    "00TGAMSTIRMI5PK4J54EIHFQ2P", "00TGAMSTIRMI5PM4J54CUF64EI",
    "00TGAMSTIRMI5PO4J54F18R2F1", "00TGAMSTIRMI5PQ4J54EANLIRU",
    "00TGAMTEBLE3R28USH3KK6SO5K", "00TGAMTEBLE3R2AUSH3LTELR5N",
    "00TGAMTEBLE3R2CUSH3NLKDIBJ", "00TGAMTEBLE3R2EUSH3LA5O8NC",
    "00TGAMTEBLE3R2GUSH3MFE3A6Q", "00TGAMTEBLE3R2IUSH3KNEMCA9",
    "00TGAMTEBLE3R2KUSH3MC38FLG", "00TGAMTEBLE3R2MUSH3N6QFG2O",
    "00TGAMTEBLE3R2OUSH3NNLOGKF", "00TGAMTEBLE3R2QUSH3KESHPMO",
    "00TGAMTEBLE3R2SUSH3KD5SLO9", "00TGAMTEBLE3R2UUSH3NLSD3BI",
    "00TGAMTEBLE3R30USH3LFC7ES4", "00TGAMTEBLE3R32USH3L1I499P",
    "00TGAMTEBLE3R34USH3M9DAEU8", "00TGAMTEBLE3R36USH3NE0ML8D",
    "00TGAMTUNCKE9JKGG5P0CAPR1A", "00TGAMTUNCKE9JMGG5P3UJ4K36",
    "00TGAMTUNNPEGS4GG5P3824I4C", "00TGAMTUNNPEGS6GG5P14QPTVP",
    "00TGAMTUNNPEGS8GG5P252U3C6", "00TGAMTUNNPEGSAGG5P3TF6BQ4",
    "00TGAMTUNNPEGSCGG5P193BG7F", "00TGAMTUNNPEGSEGG5P1PASKNU",
    "00TGAMTUNNPEGSGGG5P0BBK6EN", "00TGAMTUNNPEGSIGG5P2FGJ5KQ",
    "00TGAMTUNNPEGSKGG5P3I65THT", "00TGAMTUNNPEGSMGG5P0MDKLPJ",
    "00TGAMTUNNPEGSOGG5P3G80NS2", "00TGAMTUNNPEGSQGG5P277DSEU",
    "00TGAMTUNNPEGSSGG5P2HEPHUO", "00TGAMTUNNPEGSUGG5P0L2204P",
};

/** Encodes and decodes prepared cases correctly */
void test_encode_decode() {
  struct Case {
    uint64_t timestamp;
    uint32_t counter;
    uint32_t per_sec_random;
    uint32_t per_gen_random;
    char text[TEXT_BUFFER_SIZE];
  };

  struct Case cases[] = {
      {0, 0, 0, 0, "00000000000000000000000000"},
      {MAX_UINT44, 0, 0, 0, "7VVVVVVVVG0000000000000000"},
      {MAX_UINT44, 0, 0, 0, "7vvvvvvvvg0000000000000000"},
      {0, MAX_UINT28, 0, 0, "000000000FVVVVU00000000000"},
      {0, MAX_UINT28, 0, 0, "000000000fvvvvu00000000000"},
      {0, 0, MAX_UINT24, 0, "000000000000001VVVVS000000"},
      {0, 0, MAX_UINT24, 0, "000000000000001vvvvs000000"},
      {0, 0, 0, MAX_UINT32, "00000000000000000003VVVVVV"},
      {0, 0, 0, MAX_UINT32, "00000000000000000003vvvvvv"},
      {MAX_UINT44, MAX_UINT28, MAX_UINT24, MAX_UINT32,
       "7VVVVVVVVVVVVVVVVVVVVVVVVV"},
      {MAX_UINT44, MAX_UINT28, MAX_UINT24, MAX_UINT32,
       "7vvvvvvvvvvvvvvvvvvvvvvvvv"},
  };
  const int n_cases = sizeof(cases) / sizeof(cases[0]);

  for (int i = 0; i < n_cases; i++) {
    struct Case *e = &cases[i];
    Scru128Id from_fields, from_string;
    int err;
    err = scru128_from_fields(&from_fields, e->timestamp, e->counter,
                              e->per_sec_random, e->per_gen_random);
    assert(err == 0);
    err = scru128_from_str(&from_string, e->text);
    assert(err == 0);

    size_t text_len = strlen(e->text);
    char text_buffer[TEXT_BUFFER_SIZE];

    assert(scru128_timestamp(&from_fields) == e->timestamp);
    assert(scru128_counter(&from_fields) == e->counter);
    assert(scru128_per_sec_random(&from_fields) == e->per_sec_random);
    assert(scru128_per_gen_random(&from_fields) == e->per_gen_random);
    scru128_to_str(&from_fields, text_buffer);
    assert(strlen(text_buffer) == text_len);
    for (size_t j = 0; j < text_len; j++) {
      assert(text_buffer[j] == toupper(e->text[j]));
    }

    assert(scru128_timestamp(&from_string) == e->timestamp);
    assert(scru128_counter(&from_string) == e->counter);
    assert(scru128_per_sec_random(&from_string) == e->per_sec_random);
    assert(scru128_per_gen_random(&from_string) == e->per_gen_random);
    scru128_to_str(&from_string, text_buffer);
    assert(strlen(text_buffer) == text_len);
    for (size_t j = 0; j < text_len; j++) {
      assert(text_buffer[j] == toupper(e->text[j]));
    }
  }
}

/** Raises error if an invalid string representation is supplied */
void test_string_validation() {
  char cases[][TEXT_BUFFER_SIZE] = {
      "",
      " 00SCT4FL89GQPRHN44C4LFM0OV",
      "00SCT4FL89GQPRJN44C7SQO381 ",
      " 00SCT4FL89GQPRLN44C4BGCIIO ",
      "+00SCT4FL89GQPRNN44C4F3QD24",
      "-00SCT4FL89GQPRPN44C7H4E5RC",
      "+0SCT4FL89GQPRRN44C55Q7RVC",
      "-0SCT4FL89GQPRTN44C6PN0A2R",
      "00SCT4FL89WQPRVN44C41RGVMM",
      "00SCT4FL89GQPS1N4_C54QDC5O",
      "00SCT4-L89GQPS3N44C602O0K8",
      "00SCT4FL89GQPS N44C7VHS5QJ",
      "80000000000000000000000000",
      "VVVVVVVVVVVVVVVVVVVVVVVVVV",
  };
  const int n_cases = sizeof(cases) / sizeof(cases[0]);

  for (int i = 0; i < n_cases; i++) {
    Scru128Id x;
    int err = scru128_from_str(&x, cases[i]);
    assert(err != 0);
  }
}

/** Has symmetric converters from/to various values */
void test_symmetric_converters() {
  int n_cases = 0;
  Scru128Id cases[70];
  scru128_from_fields(&cases[n_cases++], 0, 0, 0, 0);
  scru128_from_fields(&cases[n_cases++], MAX_UINT44, 0, 0, 0);
  scru128_from_fields(&cases[n_cases++], 0, MAX_UINT28, 0, 0);
  scru128_from_fields(&cases[n_cases++], 0, 0, MAX_UINT24, 0);
  scru128_from_fields(&cases[n_cases++], 0, 0, 0, MAX_UINT32);
  scru128_from_fields(&cases[n_cases++], MAX_UINT44, MAX_UINT28, MAX_UINT24,
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
                              scru128_counter(e), scru128_per_sec_random(e),
                              scru128_per_gen_random(e));
    assert(err == 0);
    assert(scru128_compare(&id_buffer, e) == 0);
  }
}

/** Supports comparison methods */
void test_comparison_methods() {
  int n_cases = 0;
  Scru128Id ordered[73];
  scru128_from_fields(&ordered[n_cases++], 0, 0, 0, 0);
  scru128_from_fields(&ordered[n_cases++], 0, 0, 0, 1);
  scru128_from_fields(&ordered[n_cases++], 0, 0, 0, MAX_UINT32);
  scru128_from_fields(&ordered[n_cases++], 0, 0, 1, 0);
  scru128_from_fields(&ordered[n_cases++], 0, 0, MAX_UINT24, 0);
  scru128_from_fields(&ordered[n_cases++], 0, 1, 0, 0);
  scru128_from_fields(&ordered[n_cases++], 0, MAX_UINT28, 0, 0);
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

int main() {
  printf("%s:\n", __FILE__);
  run_test(test_encode_decode);
  run_test(test_string_validation);
  run_test(test_symmetric_converters);
  run_test(test_comparison_methods);
  return 0;
}
