.PHONY: all
all:
	c++ -std=c++17 -Wall -Wextra -pedantic -Werror -o stdinprinter stdinprinter.cpp 

.PHONY: clean
clean:
	rm -f stdinprinter
