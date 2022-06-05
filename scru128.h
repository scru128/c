/**
 * @file scru128.h
 *
 * SCRU128: Sortable, Clock and Random number-based Unique identifier
 *
 * @version   v0.3.0
 * @copyright Licensed under the Apache License, Version 2.0
 * @see       https://github.com/scru128/c
 */
/*
 * Copyright 2022 The scru128/c Developers.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SCRU128_H_AVJRBJQI
#define SCRU128_H_AVJRBJQI

#include <stdint.h>

/** Size in bytes of a SCRU128 ID in the binary representation (16 bytes). */
#define SCRU128_LEN (16)

/**
 * Size in bytes of a SCRU128 ID in the string representation (25 digits + NUL).
 */
#define SCRU128_STR_LEN (26)

/**
 * Represents a SCRU128 ID and provides various converters and comparison
 * operators.
 */
typedef struct Scru128Id {
  /**
   * 128-bit byte array representation in the big-endian (network) byte order.
   */
  uint8_t bytes[SCRU128_LEN];
} Scru128Id;

/**
 * @name Status codes returned by generator functions
 *
 * @{
 */

/**
 * Indicates that the latest `timestamp` was used because it was greater than
 * the previous one.
 */
#define SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP (1)

/**
 * Indicates that `counter_lo` was incremented because the latest `timestamp`
 * was no greater than the previous one.
 */
#define SCRU128_GENERATOR_STATUS_COUNTER_LO_INC (2)

/**
 * Indicates that `counter_hi` was incremented because `counter_lo` reached its
 * maximum value.
 */
#define SCRU128_GENERATOR_STATUS_COUNTER_HI_INC (3)

/**
 * Indicates that the previous `timestamp` was incremented because `counter_hi`
 * reached its maximum value.
 */
#define SCRU128_GENERATOR_STATUS_TIMESTAMP_INC (4)

/**
 * Indicates that the monotonic order of generated IDs was broken because the
 * latest `timestamp` was less than the previous one by ten seconds or more.
 */
#define SCRU128_GENERATOR_STATUS_CLOCK_ROLLBACK (5)

/** Indicates that the previous generation failed. */
#define SCRU128_GENERATOR_STATUS_ERROR (-1)

/** @} */

/**
 * Represents a SCRU128 ID generator that encapsulates the monotonic counter and
 * other internal states.
 *
 * A new generator must be initialized by `scru128_initialize_generator()`
 * before use.
 */
typedef struct Scru128Generator {
  /** @private */
  uint64_t _timestamp;

  /** @private */
  uint32_t _counter_hi;

  /** @private */
  uint32_t _counter_lo;

  /**
   * Timestamp at the last renewal of `counter_hi` field.
   *
   * @private
   */
  uint64_t _ts_counter_hi;
} Scru128Generator;

/** @private */
static const uint64_t SCRU128_MAX_TIMESTAMP = 0xffffffffffff;

/** @private */
static const uint32_t SCRU128_MAX_COUNTER_HI = 0xffffff;

/** @private */
static const uint32_t SCRU128_MAX_COUNTER_LO = 0xffffff;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Identifier-related functions
 *
 * @{
 */

/**
 * Creates a SCRU128 ID object from field values.
 *
 * @param id_out Location where the new object is stored.
 * @param timestamp 48-bit `timestamp` field value.
 * @param counter_hi 24-bit `counter_hi` field value.
 * @param counter_lo 24-bit `counter_lo` field value.
 * @param entropy 32-bit `entropy` field value.
 * @return Zero on success or a non-zero integer if any argument is out of the
 * value range of the field.
 */
static inline int scru128_from_fields(Scru128Id *id_out, uint64_t timestamp,
                                      uint32_t counter_hi, uint32_t counter_lo,
                                      uint32_t entropy) {
  if (timestamp > SCRU128_MAX_TIMESTAMP ||
      counter_hi > SCRU128_MAX_COUNTER_HI ||
      counter_lo > SCRU128_MAX_COUNTER_LO) {
    return -1;
  }

  id_out->bytes[0] = timestamp >> 40;
  id_out->bytes[1] = timestamp >> 32;
  id_out->bytes[2] = timestamp >> 24;
  id_out->bytes[3] = timestamp >> 16;
  id_out->bytes[4] = timestamp >> 8;
  id_out->bytes[5] = timestamp;
  id_out->bytes[6] = counter_hi >> 16;
  id_out->bytes[7] = counter_hi >> 8;
  id_out->bytes[8] = counter_hi;
  id_out->bytes[9] = counter_lo >> 16;
  id_out->bytes[10] = counter_lo >> 8;
  id_out->bytes[11] = counter_lo;
  id_out->bytes[12] = entropy >> 24;
  id_out->bytes[13] = entropy >> 16;
  id_out->bytes[14] = entropy >> 8;
  id_out->bytes[15] = entropy;
  return 0;
}

/**
 * Creates a SCRU128 ID object from a byte array that represents a 128-bit
 * unsigned integer.
 *
 * @param id_out Location where the new object is stored.
 * @param bytes 16-byte byte array that represents a 128-bit unsigned integer in
 * the big-endian (network) byte order.
 */
static inline void scru128_from_bytes(Scru128Id *id_out, const uint8_t *bytes) {
  for (int_fast8_t i = 0; i < SCRU128_LEN; i++) {
    id_out->bytes[i] = bytes[i];
  }
}

/**
 * Creates a SCRU128 ID object from a 25-digit string representation.
 *
 * @param id_out Location where the new object is stored.
 * @param str Null-terminated ASCII character array containing the 25-digit
 * string representation.
 * @return Zero on success or a non-zero integer if `str` is not a valid string
 * representation.
 */
static inline int scru128_from_str(Scru128Id *id_out, const char *str) {
  // O(1) map from ASCII code points to Base36 digit values.
  static const uint8_t DECODE_MAP[128] = {
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
      0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
      0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
      0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
      0x21, 0x22, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff};

  uint8_t src[25];
  for (int_fast8_t i = 0; i < 25; i++) {
    unsigned char c = str[i];
    src[i] = c > 127 ? 0xff : DECODE_MAP[c];
    if (src[i] == 0xff) {
      return -1; // invalid digit
    }
  }
  if (str[25] != 0) {
    return -1; // invalid length
  }

  for (int_fast8_t i = 0; i < SCRU128_LEN; i++) {
    id_out->bytes[i] = 0;
  }

  int_fast8_t min_index = 99; // any number greater than size of output array
  for (int_fast8_t i = -5; i < 25; i += 10) {
    // implement Base36 using 10-digit words
    uint64_t carry = 0;
    for (int_fast8_t j = i < 0 ? 0 : i; j < i + 10; j++) {
      carry = (carry * 36) + src[j];
    }

    // iterate over output array from right to left while carry != 0 but at
    // least up to place already filled
    int_fast8_t j = 15;
    for (; carry > 0 || j > min_index; j--) {
      if (j < 0) {
        return -1; // out of 128-bit value range
      }
      carry += (uint64_t)id_out->bytes[j] * 3656158440062976; // 36^10
      id_out->bytes[j] = (uint8_t)carry;
      carry = carry >> 8;
    }
    min_index = j;
  }
  return 0;
}

/** Returns the 48-bit `timestamp` field value of a SCRU128 ID. */
static inline uint64_t scru128_timestamp(const Scru128Id *id) {
  return (uint64_t)id->bytes[0] << 40 | (uint64_t)id->bytes[1] << 32 |
         (uint64_t)id->bytes[2] << 24 | (uint64_t)id->bytes[3] << 16 |
         (uint64_t)id->bytes[4] << 8 | (uint64_t)id->bytes[5];
}

/** Returns the 24-bit `counter_hi` field value of a SCRU128 ID. */
static inline uint32_t scru128_counter_hi(const Scru128Id *id) {
  return (uint32_t)id->bytes[6] << 16 | (uint32_t)id->bytes[7] << 8 |
         (uint32_t)id->bytes[8];
}

/** Returns the 24-bit `counter_lo` field value of a SCRU128 ID. */
static inline uint32_t scru128_counter_lo(const Scru128Id *id) {
  return (uint32_t)id->bytes[9] << 16 | (uint32_t)id->bytes[10] << 8 |
         (uint32_t)id->bytes[11];
}

/** Returns the 32-bit `entropy` field value of a SCRU128 ID. */
static inline uint32_t scru128_entropy(const Scru128Id *id) {
  return (uint32_t)id->bytes[12] << 24 | (uint32_t)id->bytes[13] << 16 |
         (uint32_t)id->bytes[14] << 8 | (uint32_t)id->bytes[15];
}

/**
 * Returns the 25-digit canonical string representation of a SCRU128 ID.
 *
 * @param str_out 26-byte character array where the returned string is stored.
 * The returned array is a 26-byte null-terminated string consisting of 25
 * `[0-9A-Z]` characters and null.
 */
static inline void scru128_to_str(const Scru128Id *id, char *str_out) {
  static const char DIGITS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  // zero-fill 25 elements to use in process and 26th as NUL char
  for (int_fast8_t i = 0; i < 26; i++) {
    str_out[i] = 0;
  }

  int_fast8_t min_index = 99; // any number greater than size of output array
  for (int_fast8_t i = -5; i < SCRU128_LEN; i += 7) {
    // implement Base36 using 56-bit words
    uint64_t carry = 0;
    for (int_fast8_t j = i < 0 ? 0 : i; j < i + 7; j++) {
      carry = (carry << 8) | id->bytes[j];
    }

    // iterate over output array from right to left while carry != 0 but at
    // least up to place already filled
    int_fast8_t j = 24;
    for (; carry > 0 || j > min_index; j--) {
      carry += (uint64_t)str_out[j] << 56;
      str_out[j] = carry % 36;
      carry = carry / 36;
    }
    min_index = j;
  }

  for (int_fast8_t i = 0; i < 25; i++) {
    str_out[i] = DIGITS[(unsigned char)str_out[i]];
  }
}

/**
 * Returns a negative integer, zero, or positive integer if `id_lft` is less
 * than, equal to, or greater than `id_rgt`, respectively.
 */
static inline int scru128_compare(const Scru128Id *id_lft,
                                  const Scru128Id *id_rgt) {
  for (int_fast8_t i = 0; i < SCRU128_LEN; i++) {
    if (id_lft->bytes[i] != id_rgt->bytes[i]) {
      return id_lft->bytes[i] < id_rgt->bytes[i] ? -1 : 1;
    }
  }
  return 0;
}

/** @} */

/**
 * @name Generator-related functions
 *
 * @{
 */

/** Initializes a generator struct `g`. */
static inline void scru128_initialize_generator(Scru128Generator *g) {
  g->_timestamp = 0;
  g->_counter_hi = 0;
  g->_counter_lo = 0;
  g->_ts_counter_hi = 0;
}

/**
 * Generates a new SCRU128 ID with the given `timestamp` and random number
 * generator using the generator `g`.
 *
 * @param id_out Location where the generated ID is stored.
 * @param timestamp 48-bit `timestamp` field value.
 * @param arc4random Function pointer to `arc4random()` or a compatible function
 * that returns a (cryptographically strong) random number in the range of
 * 32-bit unsigned integer.
 * @return `SCRU128_GENERATOR_STATUS_*` code that describes the characteristics
 * of generated ID. The returned code is negative if it reports an error.
 * @attention This function is NOT thread-safe. The generator `g` should be
 * protected from concurrent accesses using a mutex or other synchronization
 * mechanism to avoid race conditions.
 */
static inline int8_t scru128_generate_core(Scru128Generator *g,
                                           Scru128Id *id_out,
                                           uint64_t timestamp,
                                           uint32_t (*arc4random)(void)) {
  if (timestamp == 0 || timestamp > SCRU128_MAX_TIMESTAMP) {
    return SCRU128_GENERATOR_STATUS_ERROR;
  }

  int8_t status = SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP;
  if (timestamp > g->_timestamp) {
    g->_timestamp = timestamp;
    g->_counter_lo = (*arc4random)() & SCRU128_MAX_COUNTER_LO;
  } else if (timestamp + 10000 > g->_timestamp) {
    g->_counter_lo++;
    status = SCRU128_GENERATOR_STATUS_COUNTER_LO_INC;
    if (g->_counter_lo > SCRU128_MAX_COUNTER_LO) {
      g->_counter_lo = 0;
      g->_counter_hi++;
      status = SCRU128_GENERATOR_STATUS_COUNTER_HI_INC;
      if (g->_counter_hi > SCRU128_MAX_COUNTER_HI) {
        g->_counter_hi = 0;
        // increment timestamp at counter overflow
        g->_timestamp++;
        g->_counter_lo = (*arc4random)() & SCRU128_MAX_COUNTER_LO;
        status = SCRU128_GENERATOR_STATUS_TIMESTAMP_INC;
      }
    }
  } else {
    // reset state if clock moves back by ten seconds or more
    g->_ts_counter_hi = 0;
    g->_timestamp = timestamp;
    g->_counter_lo = (*arc4random)() & SCRU128_MAX_COUNTER_LO;
    status = SCRU128_GENERATOR_STATUS_CLOCK_ROLLBACK;
  }

  if (g->_timestamp - g->_ts_counter_hi >= 1000 || g->_ts_counter_hi == 0) {
    g->_ts_counter_hi = g->_timestamp;
    g->_counter_hi = (*arc4random)() & SCRU128_MAX_COUNTER_HI;
  }

  if (scru128_from_fields(id_out, g->_timestamp, g->_counter_hi, g->_counter_lo,
                          (*arc4random)()) == 0) {
    return status;
  } else {
    return SCRU128_GENERATOR_STATUS_ERROR;
  }
}

/** @} */

/**
 * @name High-level generator APIs that require platform integration
 *
 * @{
 */

/**
 * Generates a new SCRU128 ID using the generator `g`.
 *
 * @param id_out Location where the generated ID is stored.
 * @return `SCRU128_GENERATOR_STATUS_*` code that describes the characteristics
 * of generated ID. The returned code is negative if it reports an error.
 * @note This single-file library does not provide a concrete implementation of
 * this function, so users have to implement it to enable high-level generator
 * APIs (if necessary) by integrating the real-time clock and random number
 * generator available in the system and the `scru128_generate_core()` function.
 */
int scru128_generate(Scru128Generator *g, Scru128Id *id_out);

/**
 * Generates a new SCRU128 ID encoded in the 25-digit canonical string
 * representation using the generator `g`.
 *
 * @param str_out 26-byte character array where the returned string is stored.
 * The returned array is a 26-byte null-terminated string consisting of 25
 * `[0-9A-Z]` characters and null.
 * @return Return value of `scru128_generate()`.
 * @note Provide a concrete implementation of `scru128_generate()` to enable
 * this function.
 */
static inline int scru128_generate_string(Scru128Generator *g, char *str_out) {
  Scru128Id id;
  int status = scru128_generate(g, &id);
  if (status >= 0) {
    scru128_to_str(&id, str_out);
  }
  return status;
}

/** @} */

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* #ifndef SCRU128_H_AVJRBJQI */
