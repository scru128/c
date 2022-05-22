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

/** Maximum value of 48-bit `timestamp` field. */
static const uint64_t MAX_TIMESTAMP = 0xffffffffffff;

/** Maximum value of 24-bit `counter_hi` field. */
static const uint32_t MAX_COUNTER_HI = 0xffffff;

/** Maximum value of 24-bit `counter_lo` field. */
static const uint32_t MAX_COUNTER_LO = 0xffffff;

/** Digit characters used in the Base36 notation. */
static const char DIGITS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/** O(1) map from ASCII code points to Base36 digit values. */
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

/** Translates a big-endian byte array into uint64_t. */
static uint64_t bytes_to_uint64(const uint8_t *bytes, int nbytes) {
  uint64_t buffer = 0;
  for (int i = 0; i < nbytes; i++) {
    buffer = (buffer << 8) | bytes[i];
  }
  return buffer;
}

int scru128_from_fields(Scru128Id *out, uint64_t timestamp, uint32_t counter_hi,
                        uint32_t counter_lo, uint32_t entropy) {
  if (timestamp > MAX_TIMESTAMP || counter_hi > MAX_COUNTER_HI ||
      counter_lo > MAX_COUNTER_LO) {
    return -1;
  }

  out->_bytes[0] = (uint8_t)(timestamp >> 40);
  out->_bytes[1] = (uint8_t)(timestamp >> 32);
  out->_bytes[2] = (uint8_t)(timestamp >> 24);
  out->_bytes[3] = (uint8_t)(timestamp >> 16);
  out->_bytes[4] = (uint8_t)(timestamp >> 8);
  out->_bytes[5] = (uint8_t)timestamp;
  out->_bytes[6] = (uint8_t)(counter_hi >> 16);
  out->_bytes[7] = (uint8_t)(counter_hi >> 8);
  out->_bytes[8] = (uint8_t)counter_hi;
  out->_bytes[9] = (uint8_t)(counter_lo >> 16);
  out->_bytes[10] = (uint8_t)(counter_lo >> 8);
  out->_bytes[11] = (uint8_t)counter_lo;
  out->_bytes[12] = (uint8_t)(entropy >> 24);
  out->_bytes[13] = (uint8_t)(entropy >> 16);
  out->_bytes[14] = (uint8_t)(entropy >> 8);
  out->_bytes[15] = (uint8_t)entropy;
  return 0;
}

int scru128_from_bytes(Scru128Id *out, const uint8_t *bytes) {
  for (int i = 0; i < 16; i++) {
    out->_bytes[i] = bytes[i];
  }
  return 0;
}

int scru128_from_str(Scru128Id *out, const char *text) {
  uint8_t src[25];
  for (int i = 0; i < 25; i++) {
    unsigned char c = (unsigned char)text[i];
    if (c > 127 || DECODE_MAP[c] == 0xff) {
      return -1; // invalid digit
    }
    src[i] = DECODE_MAP[c];
  }
  if (text[25] != 0) {
    return -1; // invalid length
  }

  for (int i = 0; i < 16; i++) {
    out->_bytes[i] = 0;
  }

  int min_index = 99; // any number greater than size of output array
  for (int i = -5; i < 25; i += 10) {
    // implement Base36 using 10-digit words
    uint64_t carry = 0;
    for (int j = i < 0 ? 0 : i; j < i + 10; j++) {
      carry = (carry * 36) + src[j];
    }

    // iterate over output array from right to left while carry != 0 but at
    // least up to place already filled
    int j = 15;
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

uint64_t scru128_timestamp(const Scru128Id *id) {
  return bytes_to_uint64(&id->_bytes[0], 6);
}

uint32_t scru128_counter_hi(const Scru128Id *id) {
  return (uint32_t)bytes_to_uint64(&id->_bytes[6], 3);
}

uint32_t scru128_counter_lo(const Scru128Id *id) {
  return (uint32_t)bytes_to_uint64(&id->_bytes[9], 3);
}

uint32_t scru128_entropy(const Scru128Id *id) {
  return (uint32_t)bytes_to_uint64(&id->_bytes[12], 4);
}

void scru128_to_bytes(const Scru128Id *id, uint8_t *out) {
  for (int i = 0; i < 16; i++) {
    out[i] = id->_bytes[i];
  }
}

void scru128_to_str(const Scru128Id *id, char *out) {
  // zero-fill 25 elements to use in process and 26th as NUL char
  for (int i = 0; i < 26; i++) {
    out[i] = 0;
  }

  int min_index = 99; // any number greater than size of output array
  for (int i = -5; i < 16; i += 7) {
    // implement Base36 using 56-bit words
    uint64_t carry = i < 0 ? bytes_to_uint64(&id->_bytes[0], 2)
                           : bytes_to_uint64(&id->_bytes[i], 7);

    // iterate over output array from right to left while carry != 0 but at
    // least up to place already filled
    int j = 24;
    for (; carry > 0 || j > min_index; j--) {
      carry += (uint64_t)out[j] << 56;
      out[j] = carry % 36;
      carry = carry / 36;
    }
    min_index = j;
  }

  for (int i = 0; i < 25; i++) {
    out[i] = DIGITS[(unsigned char)out[i]];
  }
}

int scru128_compare(const Scru128Id *lhs, const Scru128Id *rhs) {
  for (int i = 0; i < 16; i++) {
    if (lhs->_bytes[i] != rhs->_bytes[i]) {
      return lhs->_bytes[i] < rhs->_bytes[i] ? -1 : 1;
    }
  }
  return 0;
}

void scru128_initialize_generator(Scru128Generator *g) {
  g->_timestamp = 0;
  g->_counter_hi = 0;
  g->_counter_lo = 0;
  g->_ts_counter_hi = 0;
  g->_last_status = SCRU128_GENERATOR_STATUS_NOT_EXECUTED;
}

Scru128GeneratorStatus scru128_generator_last_status(Scru128Generator *g) {
  return g->_last_status;
}

/**
 * Returns the current unix time in milliseconds.
 *
 * @param out Location where the returned value is stored.
 * @return Zero on success or a non-zero integer on failure.
 * @attention A concrete implementation has to be provided to build the library
 * unless the compiler flag `-DSCRU128_NO_GENERATOR` is specified.
 */
int scru128_get_msec_unixts(uint64_t *out);

/**
 * Returns a 32-bit random unsigned integer.
 *
 * @param out Location where the returned value is stored.
 * @return Zero on success or a non-zero integer on failure.
 * @attention A concrete implementation has to be provided to build the library
 * unless the compiler flag `-DSCRU128_NO_GENERATOR` is specified. Such an
 * implementation should be thread-safe and cryptographically strong.
 */
int scru128_get_random_uint32(uint32_t *out);

#ifndef SCRU128_NO_GENERATOR

int scru128_generate_core(Scru128Generator *g, Scru128Id *out,
                          uint64_t timestamp) {
  uint32_t next_rand;
  int err;

  if (timestamp > MAX_TIMESTAMP) {
    g->_last_status = SCRU128_GENERATOR_STATUS_ERROR;
    return -1;
  }

  g->_last_status = SCRU128_GENERATOR_STATUS_NEW_TIMESTAMP;
  if (timestamp > g->_timestamp) {
    g->_timestamp = timestamp;
    if ((err = scru128_get_random_uint32(&next_rand))) {
      g->_last_status = SCRU128_GENERATOR_STATUS_ERROR;
      return err;
    }
    g->_counter_lo = next_rand & MAX_COUNTER_LO;
  } else if (timestamp + 10000 > g->_timestamp) {
    g->_counter_lo++;
    g->_last_status = SCRU128_GENERATOR_STATUS_COUNTER_LO_INC;
    if (g->_counter_lo > MAX_COUNTER_LO) {
      g->_counter_lo = 0;
      g->_counter_hi++;
      g->_last_status = SCRU128_GENERATOR_STATUS_COUNTER_HI_INC;
      if (g->_counter_hi > MAX_COUNTER_HI) {
        g->_counter_hi = 0;
        // increment timestamp at counter overflow
        g->_timestamp++;
        if ((err = scru128_get_random_uint32(&next_rand))) {
          g->_last_status = SCRU128_GENERATOR_STATUS_ERROR;
          return err;
        }
        g->_counter_lo = next_rand & MAX_COUNTER_LO;
        g->_last_status = SCRU128_GENERATOR_STATUS_TIMESTAMP_INC;
      }
    }
  } else {
    // reset state if clock moves back by ten seconds or more
    g->_ts_counter_hi = 0;
    g->_timestamp = timestamp;
    if ((err = scru128_get_random_uint32(&next_rand))) {
      g->_last_status = SCRU128_GENERATOR_STATUS_ERROR;
      return err;
    }
    g->_counter_lo = next_rand & MAX_COUNTER_LO;
    g->_last_status = SCRU128_GENERATOR_STATUS_CLOCK_ROLLBACK;
  }

  if (g->_timestamp - g->_ts_counter_hi >= 1000) {
    g->_ts_counter_hi = g->_timestamp;
    if ((err = scru128_get_random_uint32(&next_rand))) {
      g->_last_status = SCRU128_GENERATOR_STATUS_ERROR;
      return err;
    }
    g->_counter_hi = next_rand & MAX_COUNTER_HI;
  }

  if ((err = scru128_get_random_uint32(&next_rand))) {
    g->_last_status = SCRU128_GENERATOR_STATUS_ERROR;
    return err;
  }
  return scru128_from_fields(out, g->_timestamp, g->_counter_hi, g->_counter_lo,
                             next_rand);
}

int scru128_generate(Scru128Generator *g, Scru128Id *out) {
  uint64_t timestamp;
  int err;
  if ((err = scru128_get_msec_unixts(&timestamp))) {
    g->_last_status = SCRU128_GENERATOR_STATUS_ERROR;
    return err;
  }
  return scru128_generate_core(g, out, timestamp);
}

int scru128_generate_string(Scru128Generator *g, char *out) {
  Scru128Id id;
  int result = scru128_generate(g, &id);
  if (result == 0) {
    scru128_to_str(&id, out);
  }
  return result;
}

#endif /* #ifndef SCRU128_NO_GENERATOR */
