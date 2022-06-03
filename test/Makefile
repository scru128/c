PLATFORM = ../platform/example_darwin.c
CFLAGS   = -I.. -Wall -Wextra -pedantic-errors -std=c99
CXXFLAGS = -I.. -Wall -Wextra -pedantic-errors -std=c++98

.PHONY: test clean test_gen test_id

test: test_gen test_id

clean:
	$(RM) *.out

test_gen: scru128_test_as_c.out scru128_test_as_cpp.out
	./scru128_test_as_c.out
	./scru128_test_as_cpp.out

scru128_test_as_c.out: ../scru128.h $(PLATFORM) scru128_test.c
	$(CC) $(CFLAGS) -o$@ $(PLATFORM) scru128_test.c

scru128_test_as_cpp.out: ../scru128.h $(PLATFORM) scru128_test.c
	$(CXX) $(CXXFLAGS) -o$@ $(PLATFORM) scru128_test.c

test_id: scru128id_test_as_c.out scru128id_test_as_cpp.out
	./scru128id_test_as_c.out
	./scru128id_test_as_cpp.out

scru128id_test_as_c.out: ../scru128.h scru128id_test.c
	$(CC) $(CFLAGS) -o$@ scru128id_test.c

scru128id_test_as_cpp.out: ../scru128.h scru128id_test.c
	$(CXX) $(CXXFLAGS) -o$@ scru128id_test.c
