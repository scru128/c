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

/**
 * Represents a SCRU128 ID generator that encapsulates the monotonic counter and
 * other internal states.
 *
 * A new generator must be initialized by `scru128_initialize_generator()`
 * before use.
 */
typedef struct Scru128Generator {
  /**
   * Timestamp at last generation.
   *
   * @private
   */
  uint64_t _ts_last_gen;

  /**
   * Counter at last generation.
   *
   * @private
   */
  uint32_t _counter;

  /**
   * Timestamp at last renewal of per_sec_random.
   *
   * @private
   */
  uint64_t _ts_last_sec;

  /**
   * Per-second random value at last generation.
   *
   * @private
   */
  uint32_t _per_sec_random;

  /**
   * Maximum number of checking the system clock until it goes forward.
   *
   * @private
   */
  int _n_clock_check_max;
} Scru128Generator;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a SCRU128 ID object from field values.
 *
 * @param out Location where the new object is stored.
 * @param timestamp 44-bit millisecond timestamp field value.
 * @param counter 28-bit per-timestamp monotonic counter field value.
 * @param per_sec_random 24-bit per-second randomness field value.
 * @param per_gen_random 32-bit per-generation randomness field value.
 * @return Zero on success or a non-zero integer if any argument is out of the
 * value range of the field.
 */
int scru128_from_fields(Scru128Id *out, uint64_t timestamp, uint32_t counter,
                        uint32_t per_sec_random, uint32_t per_gen_random);

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
 * Creates a SCRU128 ID object from a 26-digit string representation.
 *
 * @param out Location where the new object is stored.
 * @param text Null-terminated ASCII character array containing the 26-digit
 * string representation.
 * @return Zero on success or a non-zero integer if `text` is not a valid string
 * representation.
 */
int scru128_from_str(Scru128Id *out, const char *text);

/**
 * Returns the 44-bit millisecond timestamp field value of a SCRU128 ID.
 */
uint64_t scru128_timestamp(const Scru128Id *id);

/**
 * Returns the 28-bit per-timestamp monotonic counter field value of a SCRU128
 * ID.
 */
uint32_t scru128_counter(const Scru128Id *id);

/**
 * Returns the 24-bit per-second randomness field value of a SCRU128 ID.
 */
uint32_t scru128_per_sec_random(const Scru128Id *id);

/**
 * Returns the 32-bit per-generation randomness field value of a SCRU128 ID.
 */
uint32_t scru128_per_gen_random(const Scru128Id *id);

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
 * Returns the 26-digit canonical string representation of a SCRU128 ID.
 *
 * @param out Character array where the returned string is stored. The returned
 * string is a 27-byte (including the terminating null byte) ASCII string
 * consisting of 26 `[0-9A-V]` characters.
 */
void scru128_to_str(const Scru128Id *id, char *out);

/**
 * Returns a negative integer, zero, or positive integer if `lhs` is less than,
 * equal to, or greater than `rhs`, respectively.
 */
int scru128_compare(const Scru128Id *lhs, const Scru128Id *rhs);

/**
 * Initializes a generator struct `g`.
 *
 * @note This function is not available when the library is built with the
 * compiler flag `-DSCRU128_NO_GENERATOR`.
 */
void scru128_initialize_generator(Scru128Generator *g);

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
 * Generates a new SCRU128 ID encoded in the 26-digit canonical string
 * representation using the generator `g`.
 *
 * @param out Character array where the returned string is stored. The returned
 * string is a 27-byte (including the terminating null byte) ASCII string
 * consisting of 26 `[0-9A-V]` characters.
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
