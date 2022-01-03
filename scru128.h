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

/** Represents a SCRU128 ID as a 128-bit big-endian byte array. */
typedef uint8_t Scru128Id[16];

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a SCRU128 ID from field values.
 *
 * @param out Location where the created value is stored.
 * @param timestamp 44-bit millisecond timestamp field value.
 * @param counter 28-bit per-timestamp monotonic counter field value.
 * @param per_sec_random 24-bit per-second randomness field value.
 * @param per_gen_random 32-bit per-generation randomness field value.
 * @return Zero on success or a non-zero integer if any argument is out of the
 * value range of the field.
 */
int scru128_from_fields(Scru128Id out, uint64_t timestamp, uint32_t counter,
                        uint32_t per_sec_random, uint32_t per_gen_random);

/**
 * Creates a SCRU128 ID from a 26-digit string representation.
 *
 * @param out Location where the created value is stored.
 * @param text Null-terminated ASCII character array containing the 26-digit
 * string representation.
 * @return Zero on success or a non-zero integer if `text` is not a valid string
 * representation.
 */
int scru128_from_str(Scru128Id out, const char *text);

/**
 * Returns the 44-bit millisecond timestamp field value of a SCRU128 ID.
 */
uint64_t scru128_timestamp(const Scru128Id id);

/**
 * Returns the 28-bit per-timestamp monotonic counter field value of a SCRU128
 * ID.
 */
uint32_t scru128_counter(const Scru128Id id);

/**
 * Returns the 24-bit per-second randomness field value of a SCRU128 ID.
 */
uint32_t scru128_per_sec_random(const Scru128Id id);

/**
 * Returns the 32-bit per-generation randomness field value of a SCRU128 ID.
 */
uint32_t scru128_per_gen_random(const Scru128Id id);

/**
 * Returns the 26-digit canonical string representation of a SCRU128 ID.
 *
 * @param out Character array where the returned string is stored. The returned
 * string is a 27-byte (including the terminating null byte) ASCII string
 * consisting of 26 `[0-9A-V]` characters.
 */
void scru128_to_str(const Scru128Id id, char *out);

/**
 * Copies the SCRU128 ID pointed to by `src` to the location pointed to by
 * `dst`.
 */
void scru128_copy(Scru128Id dst, const Scru128Id src);

/**
 * Returns a negative integer, zero, or positive integer if `lhs` is less than,
 * equal to, or greater than `rhs`, respectively.
 */
int scru128_compare(const Scru128Id lhs, const Scru128Id rhs);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SCRU128_H_AVJRBJQI */
