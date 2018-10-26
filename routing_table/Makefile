
CC      := ${CROSS_COMPILE}gcc
CFLAGS  := -O2 -g -Wall
LIBNAME := routing_table

ifdef SPEEDUP
CARGO_FLAGS := --features "speedup"
endif

all: target/release/lib${LIBNAME}.so

test: test.o target/debug/lib${LIBNAME}.so 
	gcc -O2 $^ -o $@ -pthread -ldl

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

target/release/lib${LIBNAME}.so: src/lib.rs
	cargo build --release ${CARGO_FLAGS}

target/debug/lib${LIBNAME}.so: src/lib.rs
	cargo build ${CARGO_FLAGS}

clean:
	rm -rf test test.o
	cargo clean
