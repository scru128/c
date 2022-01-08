PLATFORM = ../platform/example_darwin.c
CFLAGS = -std=c99 -Wall -Wextra -pedantic-errors -I..
CXXFLAGS = -std=c++98 -Wall -Wextra -pedantic-errors -I..

.PHONY: test clean test_gen test_id

test: test_gen test_id

clean:
	$(RM) *.out

test_gen: scru128_test_as_c.out scru128_test_as_cpp.out
	./scru128_test_as_c.out
	./scru128_test_as_cpp.out

scru128_test_as_c.out: ../scru128.h ../scru128.c $(PLATFORM) scru128_test.c
	$(CC) $(CFLAGS) -DSCRU128_WITH_GENERATOR -o$@ ../scru128.c $(PLATFORM) scru128_test.c

scru128_test_as_cpp.out: ../scru128.h ../scru128.c $(PLATFORM) scru128_test.c
	$(CXX) $(CXXFLAGS) -DSCRU128_WITH_GENERATOR -o$@ ../scru128.c $(PLATFORM) scru128_test.c

test_id: scru128id_test_as_c.out scru128id_test_as_cpp.out
	./scru128id_test_as_c.out
	./scru128id_test_as_cpp.out

scru128id_test_as_c.out: ../scru128.h ../scru128.c scru128id_test.c
	$(CC) $(CFLAGS) -o$@ ../scru128.c scru128id_test.c

scru128id_test_as_cpp.out: ../scru128.h ../scru128.c scru128id_test.c
	$(CXX) $(CXXFLAGS) -o$@ ../scru128.c scru128id_test.c
