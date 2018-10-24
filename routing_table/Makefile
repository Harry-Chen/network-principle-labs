
CC      := ${CROSS_COMPILE}gcc
CFLAGS  := -O2 -g -Wall
LIBNAME := routing_table

all: target/release/lib${LIBNAME}.so

test: test.o target/debug/lib${LIBNAME}.so 
	gcc -O2 $^ -o $@ -pthread -ldl

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

target/release/lib${LIBNAME}.so: src/lib.rs
	cargo build --release

target/debug/lib${LIBNAME}.so: src/lib.rs
	cargo build

clean:
	rm -rf test test.o
	cargo clean
