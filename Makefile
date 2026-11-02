CC ?= gcc
AS ?= as
LD ?= ld
OBJCOPY ?= objcopy
GAME_SRC ?= games/demo.c
BUILD := build
CFLAGS := -m32 -std=c11 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-asynchronous-unwind-tables -Wall -Wextra -Werror -O2 -Iinclude

.PHONY: all clean run test FORCE
all: $(BUILD)/game.img
$(BUILD):
	mkdir -p $@
$(BUILD)/boot.o: boot/boot.S | $(BUILD)
	$(AS) --32 $< -o $@
$(BUILD)/boot.bin: $(BUILD)/boot.o
	$(LD) -m elf_i386 -Ttext 0x7c00 --oformat binary -o $@ $<
$(BUILD)/entry.o: kernel/entry.S | $(BUILD)
	$(AS) --32 $< -o $@
$(BUILD)/runtime.o: kernel/runtime.c include/game.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
# Recompile the game even when GAME_SRC changes between invocations.
$(BUILD)/game.o: FORCE $(GAME_SRC) include/game.h | $(BUILD)
	$(CC) $(CFLAGS) -c $(GAME_SRC) -o $@
$(BUILD)/kernel.elf: $(BUILD)/entry.o $(BUILD)/runtime.o $(BUILD)/game.o kernel/linker.ld
	$(LD) -m elf_i386 -T kernel/linker.ld -o $@ $(BUILD)/entry.o $(BUILD)/runtime.o $(BUILD)/game.o
$(BUILD)/kernel.bin: $(BUILD)/kernel.elf
	$(OBJCOPY) -O binary $< $@
$(BUILD)/game.img: $(BUILD)/boot.bin $(BUILD)/kernel.bin
	python3 tools/make_image.py $(BUILD)/boot.bin $(BUILD)/kernel.bin $@
run: all
	qemu-system-i386 -drive format=raw,file=$(BUILD)/game.img,if=floppy
test: all
	python3 tests/test_image.py
clean:
	rm -rf $(BUILD)
