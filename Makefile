INSTALL_PREFIX=/usr/local/bin

all: algo_test

algo_test: algo_test.c
	gcc -O3 -Wall -Wextra -w -std=gnu99 $^ -o $@

clean:
	@-rm algo_test

install: algo_test
		cp algo_test $(INSTALL_PREFIX)/algo_test
		chmod +x $(INSTALL_PREFIX)/algo_test
