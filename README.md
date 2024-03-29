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

  // generate a new identifier
  uint8_t x[16];
  scru128_generate(&g, x);
  char text[SCRU128_STR_LEN]; // 26 bytes
  scru128_to_str(x, text);
  puts(text); // e.g., "036z951mhjikzik2gsl81gr7l"

  // generate a textual representation directly
  scru128_generate_string(&g, text);
  puts(text); // e.g., "036z951mhzx67t63mq9xe6q0j"

  return 0;
}
```

See [SCRU128 Specification] for details.

[UUID]: https://en.wikipedia.org/wiki/Universally_unique_identifier
[ULID]: https://github.com/ulid/spec
[KSUID]: https://github.com/segmentio/ksuid
[SCRU128 Specification]: https://github.com/scru128/spec

## Platform integration

`scru128.h` does not provide a concrete implementation of `scru128_generate()`,
so users have to implement it to enable high-level generator APIs (if necessary)
by integrating the low-level generator primitives provided by the library with
the real-time clock and random number generator available in the system. Here is
a quick example for the BSD-like systems:

```c
#include "scru128.h"

#include <stdlib.h> // or <bsd/stdlib.h> on Linux with libbsd
#include <time.h>

int scru128_generate(Scru128Generator *g, uint8_t *id_out) {
  struct timespec tp;
  int err = clock_gettime(CLOCK_REALTIME, &tp);
  if (err) {
    return SCRU128_GENERATOR_STATUS_ERROR;
  }
  uint64_t timestamp = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  return scru128_generate_or_reset_core(g, id_out, timestamp, &arc4random,
                                        10000);
}
```

Find more examples in the [platform] directory.

[platform]: https://github.com/scru128/c/tree/main/platform

## License

Licensed under the Apache License, Version 2.0.

## See also

- [API reference](https://scru128.github.io/c/scru128_8h.html) for the list of
  provided functions
