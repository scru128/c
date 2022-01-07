CFLAGS = -std=c99 -Wall -Wextra -pedantic-errors -I..
CXXFLAGS = -std=c++98 -Wall -Wextra -pedantic-errors -I..

.PHONY: test clean test_id

test: test_id

clean:
	$(RM) *.out

test_id: scru128id_test_as_c.out scru128id_test_as_cpp.out
	./scru128id_test_as_c.out
	./scru128id_test_as_cpp.out

scru128id_test_as_c.out: ../scru128.h ../scru128.c scru128id_test.c
	$(CC) $(CFLAGS) -o$@ ../scru128.c scru128id_test.c

scru128id_test_as_cpp.out: ../scru128.h ../scru128.c scru128id_test.c
	$(CXX) $(CXXFLAGS) -o$@ ../scru128.c scru128id_test.c
