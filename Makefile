CFLAGS=-O3 -march=native -g -Wall -Wextra
LDLIBS=-lOpenCL

PROGS=main

all: $(PROGS)

clean:
	$(RM) $(PROGS)

.PHONY: all clean
