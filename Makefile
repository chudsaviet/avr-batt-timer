MCU = attiny5
CC = avr-gcc -Os -mmcu=$(MCU) -std=c23 -pedantic

firmware: build
	$(CC) src/main.c -o build/firmware.bin

build:
	mkdir build

clean:
	rm -r build