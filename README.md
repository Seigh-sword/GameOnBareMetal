# GameOnBareMetal

A small starting point for making a game that boots directly from one disk image. The image contains a BIOS bootloader, a tiny 32-bit x86 runtime, and one game linked together. The default image opens a game hub with Play, Settings, and Mode pages. Use arrows (or W/S), Enter, and Esc. The hub is a boot menu running after the machine BIOS, not a replacement BIOS. Doom is listed but cannot be launched yet.

## Build on GitHub (no local build required)

The **Build bootable image** GitHub Actions workflow runs on pushes and pull requests, or manually from the **Actions** tab. Each successful run produces exactly two artifacts: `GameOnBareMetal-IMG` (containing `game.img`) and `GameOnBareMetal-ISO` (containing `game.iso`). Both boot the same game hub; the triangle demo is tested but not published as a third artifact. GitHub Actions must be enabled for the repository. The downloads are ZIPs created by GitHub Actions; extract the image before booting it.

## Build and run locally

On a Linux host, install GNU make, binutils (with i386 support), GCC with 32-bit freestanding support, and Python 3. QEMU is needed to run the image. No 32-bit system libraries are required.

```sh
make test              # builds build/game.img and checks its layout
make run               # boots it as a floppy in qemu-system-i386
```

`build/game.img` is the complete, raw 1.44 MB floppy image. Run `make iso` (requires `xorriso`) to also create `build/game.iso`, a BIOS El Torito floppy-emulation CD image containing that same boot image. For other emulators, select a **legacy BIOS** machine and attach the IMG as a floppy or the ISO as a CD. Neither is a UEFI boot image or an installer for a hard drive; don't write it to a physical disk unless you understand that doing so destroys its contents. The image has no filesystem: the game is built into the kernel, not loaded from a separate file.

## Make a game

Write a freestanding C file that includes `game.h` and implements `void game_main(void)`. See `games/demo.c` for an example. Build it with:

```sh
make GAME_SRC=path/to/my_game.c
```

The resulting image is still `build/game.img`. The API provides an 80×25 colored VGA text screen and nonblocking raw PS/2 keyboard scancodes. Returning from `game_main` halts the machine. There is no standard library, allocator, filesystem, sound, timing, multitasking, or process isolation; game code runs with full machine privileges. The two-stage loader reads up to 512 KiB into conventional memory; kernel code, game code and static data (including BSS) must fit in that combined budget. Stage 2 checks for at least 604 KiB of conventional memory and reserves the remainder below 0x97000 for the stack. It does not yet discover or use extended RAM above 1 MiB.

## Game hub status

The hub is the default image. Play lists Doom and checks the primary IDE GmStg disk for `doom.wad` or `freedoom2.wad`, but no Doom engine is installed. Settings reports the fixed text resolution and driver status; changing resolution and updating or repairing drivers are not implemented. Mode is a placeholder, with no switchable modes yet. The old text demo is still available using `make GAME_SRC=games/demo.c`.

## Experimental software graphics

The triangle demo boots in BIOS VGA mode 13h (320×200, 256 colors) and uses a tiny software rasterizer:

```sh
make test VIDEO=graphics GAME_SRC=games/triangle.c
make run VIDEO=graphics GAME_SRC=games/triangle.c
```

The workflow also builds and tests this demo, but publishes the hub image only. `graphics_init` configures a 3:3:2 RGB palette; the caller supplies an explicit framebuffer surface to CompactGL. `compactgl.h` exposes clear, flat-colored triangles, and nearest-neighbor RGB332 texture blits in integer pixel coordinates. Texture and target buffers must not overlap. Surfaces are bounds-checked; vertices must be inside the surface. The API has no global render state, but callers must synchronize concurrent access to a shared surface. The kernel itself has no threads or multicore scheduler.

CompactGL is an early experiment inspired by lightweight explicit graphics APIs. It is **not OpenGL ES or Vulkan compatible** and does not run OpenGL ES games. Its VGA driver and renderer are software-only; it has no modern GPU driver, shaders, filtered textures, depth buffer, clipping, blending, command queues, or double buffering. Text-mode screen functions do not work in graphics mode. Modern game ports require substantially more graphics, memory and OS infrastructure.

## GammieStorage (GmStg-4098 v1)

GmStg-4098 is the format name; its capacity is **4096 MiB (4 GiB)**. It is a separate raw disk image with a small read-only bare-metal ATA PIO driver. It is not an existing filesystem, and it is not embedded in the boot IMG/ISO. The driver supports only primary IDE master/slave in legacy mode, LBA28 reads, at most 64 files, and one 512-byte sector at a time. It does not support SATA AHCI, USB, writes, directories, partition tables, or hot-plug. It is groundwork for loading game data, not a Doom driver or a working Doom port.

The storage image is not included in either boot download. To create and fill it, use Python. The disk has a 4 GiB logical size; filesystems without sparse-file support may require the full 4 GiB of free space:

```sh
python3 tools/gmstg.py create gammie-storage.img
python3 tools/gmstg.py put gammie-storage.img path/to/your.wad --name doom.wad
python3 tools/gmstg.py list gammie-storage.img
```

The image must be raw, exactly 4 GiB, and attached to the **primary IDE master** to use `gmstg_find(0, ...)`. A game can use `gmstg_read_sector` to access its file data; see `include/gmstg.h` and `tests/storage_boot.c`. For example, with QEMU add `-drive file=gammie-storage.img,format=raw,if=ide,index=0` alongside the boot floppy or CD. Keep backups: the format has no journaling or crash recovery. Only add data you have the right to use; no Doom WAD is included.

## What is a WAD?

A WAD ("Where's All the Data?") is a game-data archive containing things like maps, textures, sounds and sprites. A Doom engine needs a compatible **IWAD** (base game data); additional PWADs are optional mods and are not usually a replacement for an IWAD. The Doom engine source can be open source while the original Doom WAD remains copyrighted and distributed separately. [Freedoom](https://freedoom.github.io/) supplies freely licensed compatible game data. Neither Doom nor a WAD is included here, and adding a WAD alone will not make Doom run until the engine is ported.

## Doom roadmap

Doom is not included and cannot run yet. It needs a port of its engine and a compatible runtime, much more usable memory than the current 512 KiB static limit, input and timing, and game data (a separately licensed WAD). GmStg provides an initial way to read data but does not yet integrate with Doom. Software graphics is a first step, not an OpenGL requirement for classic Doom. A future port would need a larger image/loading design and a freestanding platform layer before it can boot.

## About other languages and graphics

This is a prototype, **not** a Vulkan or full OpenGL runner. Those APIs require substantial GPU drivers, graphics infrastructure and hardware support that this project does not yet have. It cannot run arbitrary existing games or ordinary C executables. Code must target freestanding 32-bit x86, avoid dependencies on an OS or unavailable runtime services, and be linked into the image. The build can optionally link a small `no_std` Rust component into the storage driver: install Rust with `rustup target add i686-unknown-linux-gnu`, then use `make RUST=1`. GitHub Actions builds and boot-tests this variant. The target supplies Rust core libraries; the kernel itself remains freestanding and does not run Linux. The rest of the kernel and game build is still C/assembly, and arbitrary Rust or C++ programs are not supported. Real hardware compatibility is not guaranteed.
