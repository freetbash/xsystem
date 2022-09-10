INCLUDE=-Iinclude
LIB=-Lbin
AR=ar
.PHONY:clean install uninstall test

CC=g++

bin/libxsystem.a:$(patsubst src/%.cpp,temp/%.o,$(wildcard src/*.cpp))
	$(AR) rcsv $@ $^

temp/%.o:src/%.cpp
	$(CC) -c $(INCLUDE) $^ -o $@

install:
	sudo cp include /usr/local/ -rf
	sudo cp bin/libxsystem.a /usr/local/lib/
uninstall:
	sudo rm /usr/local/include/xsystem -rf
	sudo rm /usr/local/lib/libxsystem.a

test:
	$(MAKE) -C test test

clean:
	$(RM) temp/*