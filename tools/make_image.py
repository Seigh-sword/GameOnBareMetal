#!/usr/bin/env python3
"""Assemble stage 1, stage 2 and a kernel into a 1.44 MB floppy image."""
import pathlib
import sys

boot_path, stage2_path, kernel_path, output_path = map(pathlib.Path, sys.argv[1:])
boot, stage2, kernel = (path.read_bytes() for path in (boot_path, stage2_path, kernel_path))
if len(boot) != 512 or boot[-2:] != b'\x55\xaa':
    raise SystemExit('Invalid boot sector')
if not stage2 or len(stage2) > 4 * 512:
    raise SystemExit('Stage 2 must fit in four sectors')
if not kernel or len(kernel) > 1024 * 512:
    raise SystemExit('Kernel and game exceed the 512 KiB bootloader limit')
image = boot + stage2.ljust(4 * 512, b'\0') + kernel
output_path.write_bytes(image.ljust(1440 * 1024, b'\0'))
print(f'Created {output_path} ({len(kernel)} byte kernel)')
