all: algo_test

algo_test: algo_test.c
	gcc -O3 -Wall -Wextra -w -std=gnu99 $^ -o $@

clean:
	@-rm algo_test
