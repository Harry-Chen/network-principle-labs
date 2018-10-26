SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)
HEADERS := $(SRCS:.c=.h)

EXE    := main

CC      := ${CROSS_COMPILE}gcc
CFLAGS  := -O2 -g -Wall

ifdef SPEEDUP
CFLAGS  += -DSPEEDUP
endif

LIB_NAME := routing_table

all: $(EXE)

$(EXE): $(OBJS) $(LIB_NAME)/target/release/lib$(LIB_NAME).a
	$(CC) $(CFLAGS) -o $@ $^ -pthread -ldl

$(LIB_NAME)/target/release/lib$(LIB_NAME).a:
	$(MAKE) -C $(LIB_NAME)

%.o: %.c %.h common.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf $(EXE) $(OBJS)