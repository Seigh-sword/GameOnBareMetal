CC ?= gcc
AS ?= as
LD ?= ld
OBJCOPY ?= objcopy
GAME_SRC ?= games/hub.c
VIDEO ?= text
ifeq ($(VIDEO),graphics)
VIDEO_FLAG := 1
else ifeq ($(VIDEO),text)
VIDEO_FLAG := 0
else
$(error VIDEO must be text or graphics)
endif
BUILD := build
CFLAGS := -m32 -std=c11 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-asynchronous-unwind-tables -Wall -Wextra -Werror -O2 -Iinclude

.PHONY: all clean run test iso FORCE
all: $(BUILD)/game.img
$(BUILD):
	mkdir -p $@
$(BUILD)/boot.o: boot/boot.S | $(BUILD)
	$(AS) --32 $< -o $@
$(BUILD)/boot.bin: $(BUILD)/boot.o
	$(LD) -m elf_i386 -Ttext 0x7c00 --oformat binary -o $@ $<
$(BUILD)/stage2.o: boot/stage2.S FORCE | $(BUILD)
	$(AS) --32 --defsym VGA_GRAPHICS=$(VIDEO_FLAG) $< -o $@
$(BUILD)/stage2.bin: $(BUILD)/stage2.o
	$(LD) -m elf_i386 -Ttext 0x8000 --oformat binary -o $@ $<
$(BUILD)/entry.o: kernel/entry.S | $(BUILD)
	$(AS) --32 $< -o $@
$(BUILD)/runtime.o: kernel/runtime.c include/game.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/compactgl.o: kernel/compactgl.c include/compactgl.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/hub.o: kernel/hub.c include/hub.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/gmstg.o: kernel/gmstg.c include/gmstg.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/graphics.o: kernel/graphics.c include/graphics.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
# Recompile the game even when GAME_SRC changes between invocations.
$(BUILD)/game.o: FORCE $(GAME_SRC) include/game.h | $(BUILD)
	$(CC) $(CFLAGS) -c $(GAME_SRC) -o $@
$(BUILD)/kernel.elf: $(BUILD)/entry.o $(BUILD)/runtime.o $(BUILD)/compactgl.o $(BUILD)/graphics.o $(BUILD)/gmstg.o $(BUILD)/hub.o $(BUILD)/game.o kernel/linker.ld
	$(LD) -m elf_i386 -T kernel/linker.ld -o $@ $(BUILD)/entry.o $(BUILD)/runtime.o $(BUILD)/compactgl.o $(BUILD)/graphics.o $(BUILD)/gmstg.o $(BUILD)/hub.o $(BUILD)/game.o
$(BUILD)/kernel.bin: $(BUILD)/kernel.elf
	$(OBJCOPY) -O binary $< $@
$(BUILD)/game.img: $(BUILD)/boot.bin $(BUILD)/stage2.bin $(BUILD)/kernel.bin
	python3 tools/make_image.py $(BUILD)/boot.bin $(BUILD)/stage2.bin $(BUILD)/kernel.bin $@
iso: all
	mkdir -p $(BUILD)/iso-root
	cp $(BUILD)/game.img $(BUILD)/iso-root/boot.img
	xorriso -as mkisofs -quiet -V GAMEON -b boot.img -o $(BUILD)/game.iso $(BUILD)/iso-root
run: all
	qemu-system-i386 -drive format=raw,file=$(BUILD)/game.img,if=floppy
test: all
	python3 tests/test_image.py
	$(CC) -std=c11 -Wall -Wextra -Werror -Iinclude tests/test_compactgl.c kernel/compactgl.c -o $(BUILD)/test_compactgl
	$(BUILD)/test_compactgl
	$(CC) -std=c11 -Wall -Wextra -Werror -Iinclude tests/test_hub.c kernel/hub.c -o $(BUILD)/test_hub
	$(BUILD)/test_hub
	python3 tests/test_gmstg.py
clean:
	rm -rf $(BUILD)
