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
  scru128_initialize_generator(&g);

  // generate a new identifier object
  Scru128Id x;
  scru128_generate(&g, &x);
  char text[26];
  scru128_to_str(&x, text);
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

## Build

`scru128.h` and `scru128.c` currently do not include any platform-dependent code
(they depend on `stdint.h` only) and thus platform-specific functions to access
to the system clocks and random number generators have to be implemented
separately. Define the following two functions in a separate source file and
link it to `scru128.c` at build time. Examples are found in the [platform]
directory.

```c
/**
 * Returns the current unix time in milliseconds.
 *
 * @param out Location where the returned value is stored.
 * @return Zero on success or a non-zero integer on failure.
 */
int scru128_get_msec_unixts(uint64_t *out);

/**
 * Returns a 32-bit random unsigned integer.
 *
 * @param out Location where the returned value is stored.
 * @return Zero on success or a non-zero integer on failure.
 */
int scru128_get_random_uint32(uint32_t *out);
```

Alternatively, you can specify the compiler flag `-DSCRU128_NO_GENERATOR` to
build `scru128.c` without generator functionality.

[platform]: https://github.com/scru128/c/tree/main/platform

## License

Licensed under the Apache License, Version 2.0.

## See also

- [Doxygen generated docs](https://scru128.github.io/c/scru128_8h.html) for
  provided functions
