/*
 * scru128.c - https://github.com/scru128/c
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

#include "scru128.h"

/** Maximum value of 44-bit timestamp field. */
static const uint64_t MAX_TIMESTAMP = 0xfffffffffff;

/** Maximum value of 28-bit counter field. */
static const uint32_t MAX_COUNTER = 0xfffffff;

/** Maximum value of 24-bit per_sec_random field. */
static const uint32_t MAX_PER_SEC_RANDOM = 0xffffff;

/** Digit characters used in the base 32 notation. */
static const char DIGITS[33] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

/** O(1) map from ASCII code points to base 32 digit values. */
static const uint8_t DECODE_MAP[256] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
    0x1d, 0x1e, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff};

/** Translates a big-endian byte array into uint64_t. */
static uint64_t bytes_to_uint64(const uint8_t *bytes, int nbytes) {
  uint64_t buffer = 0;
  for (int i = 0; i < nbytes; i++) {
    buffer <<= 8;
    buffer |= bytes[i];
  }
  return buffer;
}

int scru128_from_fields(Scru128Id *out, uint64_t timestamp, uint32_t counter,
                        uint32_t per_sec_random, uint32_t per_gen_random) {
  if (timestamp > MAX_TIMESTAMP || counter > MAX_COUNTER ||
      per_sec_random > MAX_PER_SEC_RANDOM) {
    return -1;
  }

  out->_bytes[0] = (uint8_t)(timestamp >> 36);
  out->_bytes[1] = (uint8_t)(timestamp >> 28);
  out->_bytes[2] = (uint8_t)(timestamp >> 20);
  out->_bytes[3] = (uint8_t)(timestamp >> 12);
  out->_bytes[4] = (uint8_t)(timestamp >> 4);
  out->_bytes[5] = (uint8_t)(timestamp << 4) | (uint8_t)(counter >> 24);
  out->_bytes[6] = (uint8_t)(counter >> 16);
  out->_bytes[7] = (uint8_t)(counter >> 8);
  out->_bytes[8] = (uint8_t)counter;
  out->_bytes[9] = (uint8_t)(per_sec_random >> 16);
  out->_bytes[10] = (uint8_t)(per_sec_random >> 8);
  out->_bytes[11] = (uint8_t)per_sec_random;
  out->_bytes[12] = (uint8_t)(per_gen_random >> 24);
  out->_bytes[13] = (uint8_t)(per_gen_random >> 16);
  out->_bytes[14] = (uint8_t)(per_gen_random >> 8);
  out->_bytes[15] = (uint8_t)per_gen_random;
  return 0;
}

int scru128_from_bytes(Scru128Id *out, const uint8_t *bytes) {
  for (int i = 0; i < 16; i++) {
    out->_bytes[i] = bytes[i];
  }
  return 0;
}

int scru128_from_str(Scru128Id *out, const char *text) {
  const uint8_t *uchars = (const uint8_t *)text;
  if (DECODE_MAP[uchars[0]] > 7 || DECODE_MAP[uchars[1]] == 0xff) {
    return -1;
  }

  out->_bytes[0] =
      (uint8_t)(DECODE_MAP[uchars[0]] << 5) | DECODE_MAP[uchars[1]];

  // process three 40-bit (5-byte / 8-digit) groups
  for (int i = 0; i < 3; i++) {
    uint64_t buffer = 0;
    for (int j = 0; j < 8; j++) {
      const uint8_t n = DECODE_MAP[uchars[2 + i * 8 + j]];
      if (n == 0xff) {
        return -1;
      }
      buffer <<= 5;
      buffer |= n;
    }
    for (int j = 0; j < 5; j++) {
      out->_bytes[5 + i * 5 - j] = (uint8_t)buffer;
      buffer >>= 8;
    }
  }
  return uchars[26] == 0 ? 0 : -1;
}

uint64_t scru128_timestamp(const Scru128Id *id) {
  return bytes_to_uint64(&id->_bytes[0], 6) >> 4;
}

uint32_t scru128_counter(const Scru128Id *id) {
  return (uint32_t)bytes_to_uint64(&id->_bytes[5], 4) & MAX_COUNTER;
}

uint32_t scru128_per_sec_random(const Scru128Id *id) {
  return (uint32_t)bytes_to_uint64(&id->_bytes[9], 3);
}

uint32_t scru128_per_gen_random(const Scru128Id *id) {
  return (uint32_t)bytes_to_uint64(&id->_bytes[12], 4);
}

void scru128_to_bytes(const Scru128Id *id, uint8_t *out) {
  for (int i = 0; i < 16; i++) {
    out[i] = id->_bytes[i];
  }
}

void scru128_to_str(const Scru128Id *id, char *out) {
  out[0] = DIGITS[id->_bytes[0] >> 5];
  out[1] = DIGITS[id->_bytes[0] & 31];

  // process three 40-bit (5-byte / 8-digit) groups
  for (int i = 0; i < 3; i++) {
    uint64_t buffer = bytes_to_uint64(&id->_bytes[1 + i * 5], 5);
    for (int j = 0; j < 8; j++) {
      out[9 + i * 8 - j] = DIGITS[buffer & 31];
      buffer >>= 5;
    }
  }

  out[26] = 0;
}

int scru128_compare(const Scru128Id *lhs, const Scru128Id *rhs) {
  for (int i = 0; i < 16; i++) {
    if (lhs->_bytes[i] != rhs->_bytes[i]) {
      return lhs->_bytes[i] < rhs->_bytes[i] ? -1 : 1;
    }
  }
  return 0;
}
