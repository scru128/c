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
  static const uint32_t MAX_COUNTER_HI = 0xffffff;
  static const uint32_t MAX_COUNTER_LO = 0xffffff;
  uint32_t next_rand;
  int err;

  if (timestamp == 0 || timestamp > 0xffffffffffff) {
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

  if (g->_timestamp - g->_ts_counter_hi >= 1000 || g->_ts_counter_hi == 0) {
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
