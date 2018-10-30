SRCS    := $(wildcard *.c)
HEADERS := $(SRCS:.c=.h)
OBJ     := obj
OBJS    := $(addprefix $(OBJ)/,$(SRCS:.c=.o))

EXE    := forwarder

CC      := ${CROSS_COMPILE}gcc
CFLAGS  := -Wall -O3

CFLAGS_RELEASE := -Wl,--strip-all -static-libstdc++ -static-libgcc -static
CFLAGS_DEBUG   := -g

LIB_NAME := routing_table


all: $(OBJ) $(OBJ)/$(EXE).release $(OBJ)/$(EXE).debug

$(OBJ):
	mkdir -p $@

$(OBJ)/$(EXE).release: $(OBJS) $(OBJ)/lib$(LIB_NAME)_release.a
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) -o $@ $^ -pthread -ldl

$(OBJ)/$(EXE).debug: $(OBJS) $(OBJ)/lib$(LIB_NAME)_debug.a
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) -o $@ $^ -pthread -ldl

$(OBJ)/lib$(LIB_NAME)_release.a: $(LIB_NAME)/target/release/lib$(LIB_NAME).a
	cp $^ $@

$(OBJ)/lib$(LIB_NAME)_debug.a: $(LIB_NAME)/target/debug/lib$(LIB_NAME).a
	cp $^ $@

$(LIB_NAME)/target/debug/lib$(LIB_NAME).a:
	$(MAKE) -C $(LIB_NAME) debug

$(LIB_NAME)/target/release/lib$(LIB_NAME).a:
	$(MAKE) -C $(LIB_NAME) release

$(OBJ)/%.o: %.c %.h common.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/main.o: main.c common.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf $(OBJ)
	$(MAKE) -C $(LIB_NAME) clean