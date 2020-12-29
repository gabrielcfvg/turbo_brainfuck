CXXFLAGS = -Wextra -Wall -pedantic -std=c++17
FILES = *pp
SOURCE = src
CC = g++

debug: $(SOURCE)/$(FILES) Makefile
	$(CC) -g -o main $(SOURCE)/*cpp $(CXXFLAGS)

asan: $(SOURCE)/$(FILES) Makefile
	$(CC) -g -fsanitize=address -o main $(SOURCE)/*cpp $(CXXFLAGS)

release: $(SOURCE)/$(FILES) Makefile
	$(CC) -O3 -o main $(SOURCE)/*cpp $(CXXFLAGS)

clean:
	rm -rf main
all: debug

.PHONY: debug, asan, release, clean, all