INCLUDE=-I../include
LIB=-L../bin
LIB_FLAG=-lxsystem
CC=g++
SRC=$(wildcard *.cpp)

.PHONY:test

test:
	@for c in $(SRC);do\
		$(CC) $(INCLUDE) $(LIB) $$c -o $$c.out $(LIB_FLAG);\
		$$c.out;\
	done;

