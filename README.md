# GameOnBareMetal

A small starting point for making a game that boots directly from one disk image. The image contains a BIOS bootloader, a tiny 32-bit x86 runtime, and one game linked together. The included demo displays a message and lets you move `@` with the arrow keys or WASD.

## Build on GitHub (no local build required)

The **Build bootable image** GitHub Actions workflow runs on pushes and pull requests. You can also start it manually from the repository's **Actions** tab: select the workflow, choose **Run workflow**, and select a branch. When the run succeeds, download the `GameOnBareMetal-image` artifact from the run page and extract `game.img` from the ZIP. To build your own game, add or edit its C source in the repository and change the `GAME_SRC` setting in the workflow's build command (for example, `make test GAME_SRC=games/my_game.c`). GitHub Actions must be enabled for the repository.

## Build and run locally

On a Linux host, install GNU make, binutils (with i386 support), GCC with 32-bit freestanding support, and Python 3. QEMU is needed to run the image. No 32-bit system libraries are required.

```sh
make test              # builds build/game.img and checks its layout
make run               # boots it as a floppy in qemu-system-i386
```

`build/game.img` is the complete, raw 1.44 MB floppy image. For other emulators, select a **legacy BIOS** machine and attach it as a floppy. This is not an ISO or a UEFI boot image; don't write it to a physical disk unless you understand that doing so destroys its contents. The image has no filesystem: the game is built into the kernel, not loaded from a separate file.

## Make a game

Write a freestanding C file that includes `game.h` and implements `void game_main(void)`. See `games/demo.c` for an example. Build it with:

```sh
make GAME_SRC=path/to/my_game.c
```

The resulting image is still `build/game.img`. The API provides an 80×25 colored VGA text screen and nonblocking raw PS/2 keyboard scancodes. Returning from `game_main` halts the machine. There is no standard library, allocator, filesystem, sound, timing, multitasking, or process isolation; game code runs with full machine privileges. The bootloader reads 64 sectors, so the kernel, game, and static memory are limited to 32 KiB combined.

## About other languages and graphics

This is a prototype, **not** a Vulkan or OpenGL runner. Those APIs require substantial GPU drivers, graphics infrastructure and hardware support that this project does not yet have. It cannot run arbitrary existing games or ordinary C executables. Code must target freestanding 32-bit x86, avoid dependencies on an OS or unavailable runtime services, and be linked into the image. Rust or C++ could eventually be used with a suitable freestanding toolchain and C-compatible entry point, but the supplied build currently supports C only. Real hardware compatibility is not guaranteed.
