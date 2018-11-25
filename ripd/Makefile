SRCS    := $(wildcard *.c)
HEADERS := $(SRCS:.c=.h)
OBJ     := obj
OBJS    := $(addprefix $(OBJ)/,$(SRCS:.c=.o))

EXE    := ripd

CC      := ${CROSS_COMPILE}gcc
CFLAGS  := -Wall -O3

CFLAGS_RELEASE := -Wl,--strip-all
CFLAGS_DEBUG   := -g


all: $(OBJ) $(OBJ)/$(EXE).release $(OBJ)/$(EXE).debug

$(OBJ):
	mkdir -p $@

$(OBJ)/$(EXE).release: $(OBJS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) -o $@ $^ -pthread -ldl

$(OBJ)/$(EXE).debug: $(OBJS)
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) -o $@ $^ -pthread -ldl

$(OBJ)/%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/main.o: main.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf $(OBJ)