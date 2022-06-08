# SCRU128: Sortable, Clock and Random number-based Unique identifier

[![GitHub tag](https://img.shields.io/github/v/tag/scru128/c)](https://github.com/scru128/c)
[![License](https://img.shields.io/github/license/scru128/c)](https://github.com/scru128/c/blob/main/LICENSE)

SCRU128 ID is yet another attempt to supersede [UUID] for the users who need
decentralized, globally unique time-ordered identifiers. SCRU128 is inspired by
[ULID] and [KSUID] and has the following features:

- 128-bit unsigned integer type
- Sortable by generation time (as integer and as text)
- 25-digit case-insensitive textual representation (Base36)
- 48-bit millisecond Unix timestamp that ensures useful life until year 10889
- Up to 281 trillion time-ordered but unpredictable unique IDs per millisecond
- 80-bit three-layer randomness for global uniqueness

```c
#include "scru128.h"

#include <stdio.h>

int main() {
  Scru128Generator g;
  scru128_generator_init(&g);

  // generate a new identifier object
  uint8_t x[SCRU128_LEN];
  scru128_generate(&g, x);
  char text[SCRU128_STR_LEN];
  scru128_to_str(x, text);
  puts(text); // e.g. "036Z951MHJIKZIK2GSL81GR7L"

  // generate a textual representation directly
  scru128_generate_string(&g, text);
  puts(text); // e.g. "036Z951MHZX67T63MQ9XE6Q0J"

  return 0;
}
```

See [SCRU128 Specification] for details.

[uuid]: https://en.wikipedia.org/wiki/Universally_unique_identifier
[ulid]: https://github.com/ulid/spec
[ksuid]: https://github.com/segmentio/ksuid
[scru128 specification]: https://github.com/scru128/spec

## Platform integration

`scru128.h` does not provide a concrete implementation of `scru128_generate()`,
so users have to implement it to enable high-level generator APIs (if necessary)
by integrating the real-time clock and random number generator available in the
system and the `scru128_generate_core()` function. Here is a quick example for
the BSD-like systems:

```c
#include "scru128.h"

#include <stdlib.h> // or <bsd/stdlib.h> on Linux with libbsd
#include <time.h>

/** @warning This example is NOT thread-safe. */
int scru128_generate(Scru128Generator *g, uint8_t *id_out) {
  struct timespec tp;
  int err = clock_gettime(CLOCK_REALTIME, &tp);
  if (err) {
    return SCRU128_GENERATOR_STATUS_ERROR;
  }
  uint64_t timestamp = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  return scru128_generate_core(g, id_out, timestamp, &arc4random);
}
```

Find more examples in the [platform] directory.

[platform]: https://github.com/scru128/c/tree/main/platform

## License

Licensed under the Apache License, Version 2.0.

## See also

- [Doxygen generated docs](https://scru128.github.io/c/scru128_8h.html) for
  provided functions
