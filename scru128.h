/**
 * @file scru128.h
 *
 * SCRU128: Sortable, Clock and Random number-based Unique identifier
 *
 * @version   v0.3.4
 * @copyright Licensed under the Apache License, Version 2.0
 * @see       https://github.com/scru128/c
 */
/*
 * Copyright 2022-2023 The scru128/c Developers.
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

/**
 * The size in bytes of a SCRU128 ID in the binary representation (16 bytes).
 */
#define SCRU128_LEN (16)

/**
 * The size in bytes of a SCRU128 ID in the string representation (25 digits +
 * NUL).
 */
#define SCRU128_STR_LEN (26)

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
 * Indicates that the generator was reinitialized and the monotonic order of
 * generated IDs was broken because the latest `timestamp` was significantly
 * smaller than the previous one.
 */
#define SCRU128_GENERATOR_STATUS_ROLLBACK_RESET (5)

/** Indicates that the previous generation failed. */
#define SCRU128_GENERATOR_STATUS_ERROR (-1)

/**
 * Indicates that the previous generation was aborted because the latest
 * `timestamp` was significantly smaller than the previous one.
 */
#define SCRU128_GENERATOR_STATUS_ROLLBACK_ABORT (-2)

/** @} */

/**
 * Represents a SCRU128 ID generator that encapsulates the monotonic counter and
 * other internal states.
 *
 * A new generator must be initialized by `scru128_generator_init()` before use.
 */
typedef struct Scru128Generator {
  /** @private */
  uint64_t _timestamp;

  /** @private */
  uint32_t _counter_hi;

  /** @private */
  uint32_t _counter_lo;

  /**
   * The timestamp at the last renewal of `counter_hi` field.
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
 * This library represents a binary SCRU128 ID as a 16-byte `uint8_t` array
 * containing a 128-bit unsigned integer in the big-endian (network) byte order.
 *
 * @{
 */

/**
 * Creates a SCRU128 ID from field values.
 *
 * @param id_out A 16-byte byte array where the created SCRU128 ID is stored.
 * @param timestamp A 48-bit `timestamp` field value.
 * @param counter_hi A 24-bit `counter_hi` field value.
 * @param counter_lo A 24-bit `counter_lo` field value.
 * @param entropy A 32-bit `entropy` field value.
 * @return Zero on success or a non-zero integer if any argument is out of the
 * value range of the field.
 */
static inline int scru128_from_fields(uint8_t *id_out, uint64_t timestamp,
                                      uint32_t counter_hi, uint32_t counter_lo,
                                      uint32_t entropy) {
  if (timestamp > SCRU128_MAX_TIMESTAMP ||
      counter_hi > SCRU128_MAX_COUNTER_HI ||
      counter_lo > SCRU128_MAX_COUNTER_LO) {
    return -1;
  }

  id_out[0] = timestamp >> 40;
  id_out[1] = timestamp >> 32;
  id_out[2] = timestamp >> 24;
  id_out[3] = timestamp >> 16;
  id_out[4] = timestamp >> 8;
  id_out[5] = timestamp;
  id_out[6] = counter_hi >> 16;
  id_out[7] = counter_hi >> 8;
  id_out[8] = counter_hi;
  id_out[9] = counter_lo >> 16;
  id_out[10] = counter_lo >> 8;
  id_out[11] = counter_lo;
  id_out[12] = entropy >> 24;
  id_out[13] = entropy >> 16;
  id_out[14] = entropy >> 8;
  id_out[15] = entropy;
  return 0;
}

/**
 * Copies a SCRU128 ID from `id_src` to `id_dst`.
 *
 * @param id_dst A 16-byte byte array where the copied SCRU128 ID is stored.
 * @param id_src A 16-byte big-endian byte array that represents a SCRU128 ID.
 */
static inline void scru128_copy(uint8_t *id_dst, const uint8_t *id_src) {
  for (int_fast8_t i = 0; i < SCRU128_LEN; i++) {
    id_dst[i] = id_src[i];
  }
}

/**
 * Creates a SCRU128 ID from a 25-digit string representation.
 *
 * @param id_out A 16-byte byte array where the created SCRU128 ID is stored.
 * @param str A null-terminated character array containing the 25-digit string
 * representation.
 * @return Zero on success or a non-zero integer if `str` is not a valid string
 * representation.
 */
static inline int scru128_from_str(uint8_t *id_out, const char *str) {
  uint8_t src[25];
  for (int_fast8_t i = 0; i < 25; i++) {
    char c = str[i];
    // clang-format off
    src[i] = (  c == '0' ?  0 : c == '1' ?  1 : c == '2' ?  2 : c == '3' ?  3
              : c == '4' ?  4 : c == '5' ?  5 : c == '6' ?  6 : c == '7' ?  7
              : c == '8' ?  8 : c == '9' ?  9 : c == 'A' ? 10 : c == 'B' ? 11
              : c == 'C' ? 12 : c == 'D' ? 13 : c == 'E' ? 14 : c == 'F' ? 15
              : c == 'G' ? 16 : c == 'H' ? 17 : c == 'I' ? 18 : c == 'J' ? 19
              : c == 'K' ? 20 : c == 'L' ? 21 : c == 'M' ? 22 : c == 'N' ? 23
              : c == 'O' ? 24 : c == 'P' ? 25 : c == 'Q' ? 26 : c == 'R' ? 27
              : c == 'S' ? 28 : c == 'T' ? 29 : c == 'U' ? 30 : c == 'V' ? 31
              : c == 'W' ? 32 : c == 'X' ? 33 : c == 'Y' ? 34 : c == 'Z' ? 35
              : c == 'a' ? 10 : c == 'b' ? 11 : c == 'c' ? 12 : c == 'd' ? 13
              : c == 'e' ? 14 : c == 'f' ? 15 : c == 'g' ? 16 : c == 'h' ? 17
              : c == 'i' ? 18 : c == 'j' ? 19 : c == 'k' ? 20 : c == 'l' ? 21
              : c == 'm' ? 22 : c == 'n' ? 23 : c == 'o' ? 24 : c == 'p' ? 25
              : c == 'q' ? 26 : c == 'r' ? 27 : c == 's' ? 28 : c == 't' ? 29
              : c == 'u' ? 30 : c == 'v' ? 31 : c == 'w' ? 32 : c == 'x' ? 33
              : c == 'y' ? 34 : c == 'z' ? 35 : 0xff);
    // clang-format on
    if (src[i] == 0xff) {
      return -1; // invalid digit
    }
  }
  if (str[25] != 0) {
    return -1; // invalid length
  }

  for (int_fast8_t i = 0; i < SCRU128_LEN; i++) {
    id_out[i] = 0;
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
      carry += (uint64_t)id_out[j] * 3656158440062976; // 36^10
      id_out[j] = (uint8_t)carry;
      carry = carry >> 8;
    }
    min_index = j;
  }
  return 0;
}

/**
 * Returns the 48-bit `timestamp` field value of a SCRU128 ID.
 *
 * @param id A 16-byte big-endian byte array that represents a SCRU128 ID.
 */
static inline uint64_t scru128_timestamp(const uint8_t *id) {
  return (uint64_t)id[0] << 40 | (uint64_t)id[1] << 32 | (uint64_t)id[2] << 24 |
         (uint64_t)id[3] << 16 | (uint64_t)id[4] << 8 | (uint64_t)id[5];
}

/**
 * Returns the 24-bit `counter_hi` field value of a SCRU128 ID.
 *
 * @param id A 16-byte big-endian byte array that represents a SCRU128 ID.
 */
static inline uint32_t scru128_counter_hi(const uint8_t *id) {
  return (uint32_t)id[6] << 16 | (uint32_t)id[7] << 8 | (uint32_t)id[8];
}

/**
 * Returns the 24-bit `counter_lo` field value of a SCRU128 ID.
 *
 * @param id A 16-byte big-endian byte array that represents a SCRU128 ID.
 */
static inline uint32_t scru128_counter_lo(const uint8_t *id) {
  return (uint32_t)id[9] << 16 | (uint32_t)id[10] << 8 | (uint32_t)id[11];
}

/**
 * Returns the 32-bit `entropy` field value of a SCRU128 ID.
 *
 * @param id A 16-byte big-endian byte array that represents a SCRU128 ID.
 */
static inline uint32_t scru128_entropy(const uint8_t *id) {
  return (uint32_t)id[12] << 24 | (uint32_t)id[13] << 16 |
         (uint32_t)id[14] << 8 | (uint32_t)id[15];
}

/**
 * Returns the 25-digit canonical string representation of a SCRU128 ID.
 *
 * @param id A 16-byte big-endian byte array that represents a SCRU128 ID.
 * @param str_out A 26-byte character array where the returned string is stored.
 * The returned array is a 26-byte null-terminated string consisting of 25
 * `[0-9a-z]` characters and null.
 */
static inline void scru128_to_str(const uint8_t *id, char *str_out) {
  static const char DIGITS[] = "0123456789abcdefghijklmnopqrstuvwxyz";

  // zero-fill 25 elements to use in process and 26th as NUL char
  for (int_fast8_t i = 0; i < 26; i++) {
    str_out[i] = 0;
  }

  int_fast8_t min_index = 99; // any number greater than size of output array
  for (int_fast8_t i = -5; i < SCRU128_LEN; i += 7) {
    // implement Base36 using 56-bit words
    uint64_t carry = 0;
    for (int_fast8_t j = i < 0 ? 0 : i; j < i + 7; j++) {
      carry = (carry << 8) | id[j];
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
 *
 * @param id_lft A 16-byte big-endian byte array that represents a SCRU128 ID.
 * @param id_rgt A 16-byte big-endian byte array that represents a SCRU128 ID.
 */
static inline int scru128_compare(const uint8_t *id_lft,
                                  const uint8_t *id_rgt) {
  for (int_fast8_t i = 0; i < SCRU128_LEN; i++) {
    if (id_lft[i] != id_rgt[i]) {
      return id_lft[i] < id_rgt[i] ? -1 : 1;
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
static inline void scru128_generator_init(Scru128Generator *g) {
  g->_timestamp = 0;
  g->_counter_hi = 0;
  g->_counter_lo = 0;
  g->_ts_counter_hi = 0;
}

/**
 * Generates a new SCRU128 ID with the given `timestamp` and random number
 * generator, or returns an error upon significant timestamp rollback.
 *
 * This function returns monotonically increasing IDs unless a `timestamp`
 * provided is significantly (by `rollback_allowance` milliseconds or more)
 * smaller than the one embedded in the immediately preceding ID. If such a
 * significant clock rollback is detected, this function aborts and returns
 * `SCRU128_GENERATOR_STATUS_ROLLBACK_ABORT`.
 *
 * See `scru128_generate_or_reset_core()` for the other mode of generation.
 *
 * @param g A generator state object used to generate an ID.
 * @param id_out A 16-byte byte array where the generated SCRU128 ID is stored.
 * @param timestamp A 48-bit `timestamp` field value.
 * @param arc4random A function pointer to `arc4random()` or a compatible
 * function that returns a (cryptographically strong) random number in the range
 * of 32-bit unsigned integer.
 * @param rollback_allowance The amount of `timestamp` rollback that is
 * considered significant. A suggested value is `10000` (milliseconds).
 * @return One of `SCRU128_GENERATOR_STATUS_*` codes that describes the
 * characteristics of generated ID. A negative return code reports an error.
 * @attention This function is NOT thread-safe. The generator `g` should be
 * protected from concurrent accesses using a mutex or other synchronization
 * mechanism to avoid race conditions.
 */
static inline int8_t
scru128_generate_or_abort_core(Scru128Generator *g, uint8_t *id_out,
                               uint64_t timestamp, uint32_t (*arc4random)(void),
                               uint64_t rollback_allowance) {
  if (timestamp == 0 || timestamp > SCRU128_MAX_TIMESTAMP) {
    return SCRU128_GENERATOR_STATUS_ERROR;
  } else if (rollback_allowance > SCRU128_MAX_TIMESTAMP) {
    return SCRU128_GENERATOR_STATUS_ERROR;
  }

  int8_t status = SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP;
  if (timestamp > g->_timestamp) {
    g->_timestamp = timestamp;
    g->_counter_lo = (*arc4random)() & SCRU128_MAX_COUNTER_LO;
  } else if (timestamp + rollback_allowance > g->_timestamp) {
    // go on with previous timestamp if new one is not much smaller
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
    // abort if clock went backwards to unbearable extent
    return SCRU128_GENERATOR_STATUS_ROLLBACK_ABORT;
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

/**
 * Generates a new SCRU128 ID with the given `timestamp` and random number
 * generator, or resets the generator upon significant timestamp rollback.
 *
 * This function returns monotonically increasing IDs unless a `timestamp`
 * provided is significantly (by `rollback_allowance` milliseconds or more)
 * smaller than the one embedded in the immediately preceding ID. If such a
 * significant clock rollback is detected, this function resets the generator
 * and returns a new ID based on the given `timestamp`.
 *
 * See `scru128_generate_or_abort_core()` for the other mode of generation.
 *
 * @param g A generator state object used to generate an ID.
 * @param id_out A 16-byte byte array where the generated SCRU128 ID is stored.
 * @param timestamp A 48-bit `timestamp` field value.
 * @param arc4random A function pointer to `arc4random()` or a compatible
 * function that returns a (cryptographically strong) random number in the range
 * of 32-bit unsigned integer.
 * @param rollback_allowance The amount of `timestamp` rollback that is
 * considered significant. A suggested value is `10000` (milliseconds).
 * @return One of `SCRU128_GENERATOR_STATUS_*` codes that describes the
 * characteristics of generated ID. A negative return code reports an error.
 * @attention This function is NOT thread-safe. The generator `g` should be
 * protected from concurrent accesses using a mutex or other synchronization
 * mechanism to avoid race conditions.
 */
static inline int8_t
scru128_generate_or_reset_core(Scru128Generator *g, uint8_t *id_out,
                               uint64_t timestamp, uint32_t (*arc4random)(void),
                               uint64_t rollback_allowance) {
  int8_t status = scru128_generate_or_abort_core(
      g, id_out, timestamp, arc4random, rollback_allowance);
  if (status == SCRU128_GENERATOR_STATUS_ROLLBACK_ABORT) {
    // reset state and resume
    g->_timestamp = 0;
    g->_ts_counter_hi = 0;
    scru128_generate_or_abort_core(g, id_out, timestamp, arc4random,
                                   rollback_allowance);
    status = SCRU128_GENERATOR_STATUS_ROLLBACK_RESET;
  }
  return status;
}

/** @} */

/**
 * @name High-level generator APIs that require platform integration
 *
 * @{
 */

/**
 * Generates a new SCRU128 ID from the current `timestamp`.
 *
 * @param g A generator state object used to generate an ID.
 * @param id_out A 16-byte byte array where the generated SCRU128 ID is stored.
 * @return One of `SCRU128_GENERATOR_STATUS_*` codes that describes the
 * characteristics of generated ID. A negative return code reports an error.
 * @note This single-file library does not provide a concrete implementation of
 * this function, so users have to implement it to enable high-level generator
 * APIs (if necessary) by integrating the low-level generator primitives
 * provided by the library with the real-time clock and random number generator
 * available in the system.
 * @attention The thread-safety of this function is implementation-dependent,
 * but it is usually NOT thread-safe. The generator `g` should be protected from
 * concurrent accesses using a mutex or other synchronization mechanism to avoid
 * race conditions.
 */
int scru128_generate(Scru128Generator *g, uint8_t *id_out);

/**
 * Generates a new SCRU128 ID encoded in the 25-digit canonical string
 * representation.
 *
 * @param g A generator state object used to generate an ID.
 * @param str_out A 26-byte character array where the returned string is stored.
 * The returned array is a 26-byte null-terminated string consisting of 25
 * `[0-9a-z]` characters and null.
 * @return The return value of `scru128_generate()`.
 * @note Provide a concrete implementation of `scru128_generate()` to enable
 * this function.
 * @attention See `scru128_generate()` for the thread-safety consideration.
 */
static inline int scru128_generate_string(Scru128Generator *g, char *str_out) {
  uint8_t id[SCRU128_LEN];
  int status = scru128_generate(g, id);
  if (status >= 0) {
    scru128_to_str(id, str_out);
  }
  return status;
}

/** @} */

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* #ifndef SCRU128_H_AVJRBJQI */
