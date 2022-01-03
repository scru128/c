.PHONY: test clean test_as_c test_as_cpp

test: test_as_c test_as_cpp

clean:
	$(RM) *.out

test_as_c: scru128id_test_as_c.out
	./scru128id_test_as_c.out

test_as_cpp: scru128id_test_as_cpp.out
	./scru128id_test_as_cpp.out

%_as_c.out: %.c ../scru128.h ../scru128id.c
	$(CC) -std=c99 -Wall -Wextra -pedantic-errors -I.. -o$@ $< ../scru128id.c

%_as_cpp.out: %.c ../scru128.h ../scru128id.c
	$(CXX) -std=c++98 -Wall -Wextra -pedantic-errors -I.. -o$@ $< ../scru128id.c
