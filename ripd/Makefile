SRCS    := $(wildcard *.c)
HEADERS := $(SRCS:.c=.h)
OBJ     := obj
OBJS    := $(addprefix $(OBJ)/,$(SRCS:.c=.o))

EXE    := ripd

CC      := ${CROSS_COMPILE}gcc
CXX     := ${CROSS_COMPILE}g++
CFLAGS  := -Wall -O2
CXXFLAGS:= ${CFLAGS} -std=c++17

CFLAGS_RELEASE := -Wl,--strip-all -static-libstdc++ -static-libgcc -static
CFLAGS_DEBUG   := -g


all: $(OBJ) $(OBJ)/$(EXE).release $(OBJ)/$(EXE).debug

$(OBJ):
	mkdir -p $@

$(OBJ)/$(EXE).release: $(OBJS) $(OBJ)/rt.o
	$(CXX) $(CXXLAGS) $(CFLAGS_RELEASE) -o $@ $^ -pthread -ldl

$(OBJ)/$(EXE).debug: $(OBJS) $(OBJ)/rt.o
	$(CXX) $(CXXLAGS) $(CFLAGS_DEBUG) -o $@ $^ -pthread -ldl

$(OBJ)/%.o: %.c %.h common.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/%.o: %.c %.h common.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/main.o: main.c common.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/rt.o: rt.cpp rt.h common.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean :
	rm -rf $(OBJ)
