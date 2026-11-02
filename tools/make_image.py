#!/usr/bin/env python3
"""Assemble four BIOS stages and a kernel into a 1.44 MB floppy image."""
import pathlib
import sys

boot_path, stage2_path, stage3_path, stage4_path, kernel_path, output_path = map(pathlib.Path, sys.argv[1:])
boot, stage2, stage3, stage4, kernel = (path.read_bytes() for path in (boot_path, stage2_path, stage3_path, stage4_path, kernel_path))
if len(boot) != 512 or boot[-2:] != b'\x55\xaa':
    raise SystemExit('Invalid boot sector')
for name, stage, limit in [('Stage 2', stage2, 2048), ('Stage 3', stage3, 1024), ('Stage 4', stage4, 1024)]:
    if not stage or len(stage) > limit:
        raise SystemExit(f'{name} must fit in {limit} bytes')
if not kernel or len(kernel) > 1024 * 512:
    raise SystemExit('Kernel and game exceed the 512 KiB bootloader limit')
image = boot + stage2.ljust(2048, b'\0') + stage3.ljust(1024, b'\0') + stage4.ljust(1024, b'\0') + kernel
output_path.write_bytes(image.ljust(1440 * 1024, b'\0'))
print(f'Created {output_path} ({len(kernel)} byte kernel)')
