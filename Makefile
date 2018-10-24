SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)
HEADERS := $(SRCS:.c=.h)

EXE    := main

CC      := ${CROSS_COMPILE}gcc
CFLAGS  := -O2 -g -Wall

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -pthread

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf $(EXE) $(OBJS)