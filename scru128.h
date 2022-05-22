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
int scru128_from_fields(Scru128Id *out, uint64_t timestamp, uint32_t counter_hi,
                        uint32_t counter_lo, uint32_t entropy);

/**
 * Creates a SCRU128 ID object from a byte array that represents a 128-bit
 * unsigned integer.
 *
 * @param out Location where the new object is stored.
 * @param bytes 16-byte byte array that represents a 128-bit unsigned integer in
 * the big-endian (network) byte order.
 * @return Zero on success or a non-zero integer on failure.
 */
int scru128_from_bytes(Scru128Id *out, const uint8_t *bytes);

/**
 * Creates a SCRU128 ID object from a 25-digit string representation.
 *
 * @param out Location where the new object is stored.
 * @param text Null-terminated ASCII character array containing the 25-digit
 * string representation.
 * @return Zero on success or a non-zero integer if `text` is not a valid string
 * representation.
 */
int scru128_from_str(Scru128Id *out, const char *text);

/** Returns the 48-bit `timestamp` field value of a SCRU128 ID. */
uint64_t scru128_timestamp(const Scru128Id *id);

/** Returns the 24-bit `counter_hi` field value of a SCRU128 ID. */
uint32_t scru128_counter_hi(const Scru128Id *id);

/** Returns the 24-bit `counter_lo` field value of a SCRU128 ID. */
uint32_t scru128_counter_lo(const Scru128Id *id);

/** Returns the 32-bit `entropy` field value of a SCRU128 ID. */
uint32_t scru128_entropy(const Scru128Id *id);

/**
 * Returns a byte array containing the 128-bit unsigned integer representation
 * of a SCRU128 ID.
 *
 * @param out Unsigned byte array where the returned array is stored. The
 * returned array is a 16-byte byte array containing the 128-bit unsigned
 * integer representation in the big-endian (network) byte order.
 */
void scru128_to_bytes(const Scru128Id *id, uint8_t *out);

/**
 * Returns the 25-digit canonical string representation of a SCRU128 ID.
 *
 * @param out Character array where the returned string is stored. The returned
 * string is a 26-byte (including the terminating null byte) ASCII string
 * consisting of 25 `[0-9A-Z]` characters.
 */
void scru128_to_str(const Scru128Id *id, char *out);

/**
 * Returns a negative integer, zero, or positive integer if `lhs` is less than,
 * equal to, or greater than `rhs`, respectively.
 */
int scru128_compare(const Scru128Id *lhs, const Scru128Id *rhs);

/** Initializes a generator struct `g`. */
void scru128_initialize_generator(Scru128Generator *g);

/**
 * Returns a `Scru128GeneratorStatus` code that indicates the internal state
 * involved in the last generation of ID.
 *
 * @note The generator `g` should be protected from concurrent accesses during
 * the sequential calls to a generation function and this method to avoid race
 * conditions.
 */
Scru128GeneratorStatus scru128_generator_last_status(Scru128Generator *g);

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
