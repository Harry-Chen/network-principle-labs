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
LIB_FILENAME := lib$(LIB_NAME)
LIB_PATH := $(LIB_NAME)/target/release


all: $(OBJ) $(OBJ)/$(EXE).release $(OBJ)/$(EXE).debug

$(OBJ):
	mkdir -p $@

$(OBJ)/$(EXE).release: $(OBJS) $(OBJ)/$(LIB_FILENAME).a
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) -o $@ $^ -pthread -ldl

$(OBJ)/$(EXE).debug: $(OBJS) $(OBJ)/$(LIB_FILENAME).so
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) -o $@ $^ -pthread -ldl

$(OBJ)/$(LIB_FILENAME).a: $(LIB_PATH)/$(LIB_FILENAME).a
	cp $^ $@

$(OBJ)/$(LIB_FILENAME).so: $(LIB_PATH)/$(LIB_FILENAME).so
	cp $^ $@

$(LIB_PATH)/$(LIB_FILENAME).so: $(LIB_PATH)/$(LIB_FILENAME).a

$(LIB_PATH)/$(LIB_FILENAME).a:
	$(MAKE) -C $(LIB_NAME) release

$(OBJ)/%.o: %.c %.h common.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/main.o: main.c common.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf $(OBJ)
	$(MAKE) -C $(LIB_NAME) clean