MCU = attiny5
CC = avr-gcc -Os -mmcu=$(MCU)

firmware: build
	$(CC) src/main.c -o build/firmware.bin

build:
	mkdir build

clean:
	rm -r build