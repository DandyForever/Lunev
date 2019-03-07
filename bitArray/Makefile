CC	:= gcc
CFLAGS = -g -Wall -std=c99 $(CFLAGS.$@)
GFLAGS := -lgcov -fprofile-arcs -ftest-coverage

TRASH := *.gcda *.gcov *.gcno *.o

CFLAGS.test := $(GFLAGS)
test: test.o BitArray.o
	$(CC) $^ -o $@ $(CFLAGS)

run: test
	valgrind ./test 1>/dev/null || echo "\n"
	./test > log.txt
	gcov BitArray.gcda | grep BitArray.c -A3 -m1

CFLAGS.test.o := $(GFLAGS)
test.o: test.c BitArray.h
	$(CC) -c $< -o $@ $(CFLAGS)

CFLAGS.BitArray.o := $(GFLAGS)
BitArray.o: BitArray.c BitArray.h
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@$(RM) $(TRASH) test
