#!/usr/bin/env python3
"""Assemble a boot sector and kernel into a standard 1.44 MB floppy image."""
import pathlib
import sys

boot_path, kernel_path, output_path = map(pathlib.Path, sys.argv[1:])
boot = boot_path.read_bytes()
kernel = kernel_path.read_bytes()
if len(boot) != 512 or boot[-2:] != b'\x55\xaa':
    raise SystemExit('Invalid boot sector')
if len(kernel) > 64 * 512:
    raise SystemExit('Kernel and game exceed the 32 KiB bootloader limit')
output_path.write_bytes((boot + kernel).ljust(1440 * 1024, b'\0'))
print(f'Created {output_path} ({len(kernel)} byte kernel)')
