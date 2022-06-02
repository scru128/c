/*
 * scru128.h - https://github.com/scru128/c
 *
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

/**
 * Represents a SCRU128 ID and provides various converters and comparison
 * operators.
 */
typedef struct Scru128Id {
  /**
   * Internal 128-bit byte array representation.
   *
   * @private
   */
  uint8_t _bytes[16];
} Scru128Id;

/** Status code returned by `scru128_generator_last_status()` function. */
typedef enum Scru128GeneratorStatus {
  /** Indicates that the generator has yet to generate an ID. */
  SCRU128_GENERATOR_STATUS_NOT_EXECUTED = 0,

  /**
   * Indicates that the latest `timestamp` was used because it was greater
   * than the previous one.
   */
  SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP,

  /**
   * Indicates that `counter_lo` was incremented because the latest
   * `timestamp` was no greater than the previous one.
   */
  SCRU128_GENERATOR_STATUS_COUNTER_LO_INC,

  /**
   * Indicates that `counter_hi` was incremented because `counter_lo` reached
   * its maximum value.
   */
  SCRU128_GENERATOR_STATUS_COUNTER_HI_INC,

  /**
   * Indicates that the previous `timestamp` was incremented because
   * `counter_hi` reached its maximum value.
   */
  SCRU128_GENERATOR_STATUS_TIMESTAMP_INC,

  /**
   * Indicates that the monotonic order of generated IDs was broken because
   * the latest `timestamp` was less than the previous one by ten seconds or
   * more.
   */
  SCRU128_GENERATOR_STATUS_CLOCK_ROLLBACK,

  /** Indicates that the previous generation failed. */
  SCRU128_GENERATOR_STATUS_ERROR
} Scru128GeneratorStatus;

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

  /**
   * Status code reported at the last generation.
   *
   * @private
   */
  Scru128GeneratorStatus _last_status;
} Scru128Generator;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a SCRU128 ID object from field values.
 *
 * @param out Location where the new object is stored.
 * @param timestamp 48-bit `timestamp` field value.
 * @param counter_hi 24-bit `counter_hi` field value.
 * @param counter_lo 24-bit `counter_lo` field value.
 * @param entropy 32-bit `entropy` field value.
 * @return Zero on success or a non-zero integer if any argument is out of the
 * value range of the field.
 */
static inline int scru128_from_fields(Scru128Id *out, uint64_t timestamp,
                                      uint32_t counter_hi, uint32_t counter_lo,
                                      uint32_t entropy) {
  if (timestamp > 0xffffffffffff || counter_hi > 0xffffff ||
      counter_lo > 0xffffff) {
    return -1;
  }

  out->_bytes[0] = timestamp >> 40;
  out->_bytes[1] = timestamp >> 32;
  out->_bytes[2] = timestamp >> 24;
  out->_bytes[3] = timestamp >> 16;
  out->_bytes[4] = timestamp >> 8;
  out->_bytes[5] = timestamp;
  out->_bytes[6] = counter_hi >> 16;
  out->_bytes[7] = counter_hi >> 8;
  out->_bytes[8] = counter_hi;
  out->_bytes[9] = counter_lo >> 16;
  out->_bytes[10] = counter_lo >> 8;
  out->_bytes[11] = counter_lo;
  out->_bytes[12] = entropy >> 24;
  out->_bytes[13] = entropy >> 16;
  out->_bytes[14] = entropy >> 8;
  out->_bytes[15] = entropy;
  return 0;
}

/**
 * Creates a SCRU128 ID object from a byte array that represents a 128-bit
 * unsigned integer.
 *
 * @param out Location where the new object is stored.
 * @param bytes 16-byte byte array that represents a 128-bit unsigned integer in
 * the big-endian (network) byte order.
 * @return Zero on success or a non-zero integer on failure.
 */
static inline int scru128_from_bytes(Scru128Id *out, const uint8_t *bytes) {
  for (int_fast8_t i = 0; i < 16; i++) {
    out->_bytes[i] = bytes[i];
  }
  return 0;
}

/**
 * Creates a SCRU128 ID object from a 25-digit string representation.
 *
 * @param out Location where the new object is stored.
 * @param text Null-terminated ASCII character array containing the 25-digit
 * string representation.
 * @return Zero on success or a non-zero integer if `text` is not a valid string
 * representation.
 */
static inline int scru128_from_str(Scru128Id *out, const char *text) {
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
    unsigned char c = (unsigned char)text[i];
    if (c > 127 || DECODE_MAP[c] == 0xff) {
      return -1; // invalid digit
    }
    src[i] = DECODE_MAP[c];
  }
  if (text[25] != 0) {
    return -1; // invalid length
  }

  for (int_fast8_t i = 0; i < 16; i++) {
    out->_bytes[i] = 0;
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
      carry += (uint64_t)out->_bytes[j] * 3656158440062976; // 36^10
      out->_bytes[j] = (uint8_t)carry;
      carry = carry >> 8;
    }
    min_index = j;
  }
  return 0;
}

/** Returns the 48-bit `timestamp` field value of a SCRU128 ID. */
static inline uint64_t scru128_timestamp(const Scru128Id *id) {
  return (uint64_t)id->_bytes[0] << 40 | (uint64_t)id->_bytes[1] << 32 |
         (uint64_t)id->_bytes[2] << 24 | (uint64_t)id->_bytes[3] << 16 |
         (uint64_t)id->_bytes[4] << 8 | (uint64_t)id->_bytes[5];
}

/** Returns the 24-bit `counter_hi` field value of a SCRU128 ID. */
static inline uint32_t scru128_counter_hi(const Scru128Id *id) {
  return (uint32_t)id->_bytes[6] << 16 | (uint32_t)id->_bytes[7] << 8 |
         (uint32_t)id->_bytes[8];
}

/** Returns the 24-bit `counter_lo` field value of a SCRU128 ID. */
static inline uint32_t scru128_counter_lo(const Scru128Id *id) {
  return (uint32_t)id->_bytes[9] << 16 | (uint32_t)id->_bytes[10] << 8 |
         (uint32_t)id->_bytes[11];
}

/** Returns the 32-bit `entropy` field value of a SCRU128 ID. */
static inline uint32_t scru128_entropy(const Scru128Id *id) {
  return (uint32_t)id->_bytes[12] << 24 | (uint32_t)id->_bytes[13] << 16 |
         (uint32_t)id->_bytes[14] << 8 | (uint32_t)id->_bytes[15];
}

/**
 * Returns a byte array containing the 128-bit unsigned integer representation
 * of a SCRU128 ID.
 *
 * @param out Unsigned byte array where the returned array is stored. The
 * returned array is a 16-byte byte array containing the 128-bit unsigned
 * integer representation in the big-endian (network) byte order.
 */
static inline void scru128_to_bytes(const Scru128Id *id, uint8_t *out) {
  for (int_fast8_t i = 0; i < 16; i++) {
    out[i] = id->_bytes[i];
  }
}

/**
 * Returns the 25-digit canonical string representation of a SCRU128 ID.
 *
 * @param out Character array where the returned string is stored. The returned
 * string is a 26-byte (including the terminating null byte) ASCII string
 * consisting of 25 `[0-9A-Z]` characters.
 */
static inline void scru128_to_str(const Scru128Id *id, char *out) {
  static const char DIGITS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  // zero-fill 25 elements to use in process and 26th as NUL char
  for (int_fast8_t i = 0; i < 26; i++) {
    out[i] = 0;
  }

  int_fast8_t min_index = 99; // any number greater than size of output array
  for (int_fast8_t i = -5; i < 16; i += 7) {
    // implement Base36 using 56-bit words
    uint64_t carry = 0;
    for (int_fast8_t j = i < 0 ? 0 : i; j < i + 7; j++) {
      carry = (carry << 8) | id->_bytes[j];
    }

    // iterate over output array from right to left while carry != 0 but at
    // least up to place already filled
    int_fast8_t j = 24;
    for (; carry > 0 || j > min_index; j--) {
      carry += (uint64_t)out[j] << 56;
      out[j] = carry % 36;
      carry = carry / 36;
    }
    min_index = j;
  }

  for (int_fast8_t i = 0; i < 25; i++) {
    out[i] = DIGITS[(unsigned char)out[i]];
  }
}

/**
 * Returns a negative integer, zero, or positive integer if `lhs` is less than,
 * equal to, or greater than `rhs`, respectively.
 */
static inline int scru128_compare(const Scru128Id *lhs, const Scru128Id *rhs) {
  for (int_fast8_t i = 0; i < 16; i++) {
    if (lhs->_bytes[i] != rhs->_bytes[i]) {
      return lhs->_bytes[i] < rhs->_bytes[i] ? -1 : 1;
    }
  }
  return 0;
}

/** Initializes a generator struct `g`. */
static inline void scru128_initialize_generator(Scru128Generator *g) {
  g->_timestamp = 0;
  g->_counter_hi = 0;
  g->_counter_lo = 0;
  g->_ts_counter_hi = 0;
  g->_last_status = SCRU128_GENERATOR_STATUS_NOT_EXECUTED;
}

/**
 * Returns a `Scru128GeneratorStatus` code that indicates the internal state
 * involved in the last generation of ID.
 *
 * @note The generator `g` should be protected from concurrent accesses during
 * the sequential calls to a generation function and this method to avoid race
 * conditions.
 */
static inline Scru128GeneratorStatus
scru128_generator_last_status(Scru128Generator *g) {
  return g->_last_status;
}

/**
 * Generates a new SCRU128 ID with the `timestamp` passed using the generator
 * `g`.
 *
 * @param out Location where the generated ID is stored.
 * @param timestamp 48-bit `timestamp` field value.
 * @return Zero on success or a non-zero integer if `timestamp` is not a 48-bit
 * positive integer or the random number generator returns an error.
 * @attention This function is NOT thread-safe. The generator `g` should be
 * protected from concurrent accesses using a mutex or other synchronization
 * mechanism to avoid race conditions.
 * @note This function is not available when the library is built with the
 * compiler flag `-DSCRU128_NO_GENERATOR`.
 */
int scru128_generate_core(Scru128Generator *g, Scru128Id *out,
                          uint64_t timestamp);

/**
 * Generates a new SCRU128 ID using the generator `g`.
 *
 * @param out Location where the generated ID is stored.
 * @return Zero on success or a non-zero integer if the system clock or random
 * number generator returns an error.
 * @attention This function is NOT thread-safe. The generator `g` should be
 * protected from concurrent accesses using a mutex or other synchronization
 * mechanism to avoid race conditions.
 * @note This function is not available when the library is built with the
 * compiler flag `-DSCRU128_NO_GENERATOR`.
 */
int scru128_generate(Scru128Generator *g, Scru128Id *out);

/**
 * Generates a new SCRU128 ID encoded in the 25-digit canonical string
 * representation using the generator `g`.
 *
 * @param out Character array where the returned string is stored. The returned
 * string is a 26-byte (including the terminating null byte) ASCII string
 * consisting of 25 `[0-9A-Z]` characters.
 * @return Zero on success or a non-zero integer if the system clock or random
 * number generator returns an error.
 * @attention This function is NOT thread-safe. The generator `g` should be
 * protected from concurrent accesses using a mutex or other synchronization
 * mechanism to avoid race conditions.
 * @note This function is not available when the library is built with the
 * compiler flag `-DSCRU128_NO_GENERATOR`.
 */
int scru128_generate_string(Scru128Generator *g, char *out);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* #ifndef SCRU128_H_AVJRBJQI */
