.PHONY: all
all: run

.PHONY: run
run: stdinprinter
	./stdinprinter

stdinprinter: stdinprinter.cpp
	c++ -std=c++17 -Wall -Wextra -pedantic -Werror -o stdinprinter stdinprinter.cpp

.PHONY: clean
clean:
	rm -f stdinprinter
