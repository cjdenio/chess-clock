all: a.bin

a.out: main.c
	avr-gcc -g -Os -mmcu=attiny84a $< -o $@

a.bin: a.out
	avr-objcopy -O binary $< $@

deploy: a.bin
	avrdude -p t84a -c stk500v1 -P/dev/tty.usbmodemflip_Rab3gao3 -U flash:w:$<:r
