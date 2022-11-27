CC=/usr/bin/gcc
CFLAGS=-std=gnu11 -O3

all: jetson_mini_fan

jetson_mini_fan: jetson_mini_fan.c
	${CC} ${CFLAGS} $^ -o $@ 

clean:
	rm -rf ./jetson_mini_fan

install: jetson_mini_fan
	chmod +x ./install.sh
	./install.sh

uninstall:
	chmod +x ./uninstall.sh
	./uninstall.sh
