CC=/usr/bin/gcc
CFLAGS=-std=gnu11 -O3

all: jetson_mini_fan

jetson_mini_fan: jetson_mini_fan.c
	${CC} ${CFLAGS} $^ -o $@ 

clean:
	rm -rf ./jetson_mini_fan

install: jetson_mini_fan
	sudo chmod +x ./install.sh
	sudo ./install.sh

uninstall:
	sudo chmod +x ./uninstall.sh
	sudo ./uninstall.sh
