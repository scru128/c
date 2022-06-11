PLATFORM = ../platform/example_bsd.c
CFLAGS   = -I.. -Wall -Wextra -pedantic-errors -std=c99
CXXFLAGS = -I.. -Wall -Wextra -pedantic-errors -std=c++98

.PHONY: test clean test_gen test_core

test: test_gen test_core

clean:
	$(RM) *.out

test_gen: scru128_test_as_c.out scru128_test_as_cpp.out
	./scru128_test_as_c.out
	./scru128_test_as_cpp.out

scru128_test_as_c.out: ../scru128.h $(PLATFORM) scru128_test.c
	$(CC) $(CFLAGS) -o$@ $(PLATFORM) scru128_test.c

scru128_test_as_cpp.out: ../scru128.h $(PLATFORM) scru128_test.c
	$(CXX) $(CXXFLAGS) -o$@ $(PLATFORM) scru128_test.c

test_core: scru128_test_core_as_c.out scru128_test_core_as_cpp.out
	./scru128_test_core_as_c.out
	./scru128_test_core_as_cpp.out

scru128_test_core_as_c.out: ../scru128.h scru128_test_core.c
	$(CC) $(CFLAGS) -o$@ scru128_test_core.c

scru128_test_core_as_cpp.out: ../scru128.h scru128_test_core.c
	$(CXX) $(CXXFLAGS) -o$@ scru128_test_core.c
