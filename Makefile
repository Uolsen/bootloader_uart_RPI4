# ARMGNU ?= arm-none-eabi
# ARMGNU ?= aarch64-linux-gnu
ARMGNU ?= aarch64-none-elf

RPI ?= RPI4B

COPS = -Wall -D $(RPI) -O2 -nostdlib -nostartfiles -ffreestanding

all : kernel.img
all4 : kernel4.img

clean :
	rm -f *.o
	rm -f *.bin
	rm -f *.hex
	rm -f *.elf
	rm -f *.list
	rm -f *.img

vectors.o : vectors.s
	$(ARMGNU)-as vectors.s -o vectors.o

vectors4.o : vectors4.s
	$(ARMGNU)-as -c vectors4.s -o vectors4.o

loader.o: loader.c
	$(ARMGNU)-gcc $(COPS) -c loader.c -o loader.o

peripherals.o: peripherals.c 
	$(ARMGNU)-gcc $(COPS) -c peripherals.c -o peripherals.o

kernel.img : link.ld vectors.o peripherals.o loader.o 
	$(ARMGNU)-ld vectors.o peripherals.o loader.o -T link.ld -o loader.elf
	$(ARMGNU)-objdump -D loader.elf > loader.list
	$(ARMGNU)-objcopy loader.elf -O binary kernel.img

kernel4.img : clean link4.ld vectors4.o peripherals.o loader.o
	$(ARMGNU)-ld vectors4.o peripherals.o loader.o -T link4.ld -o loader.elf
	$(ARMGNU)-objdump -D loader.elf > loader.list
	$(ARMGNU)-objcopy loader.elf -O binary kernel8.img