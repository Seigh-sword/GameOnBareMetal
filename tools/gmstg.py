#!/usr/bin/env python3
"""Create and populate a Gm-Stg16 v1 disk image (16 GiB logical size)."""
import argparse
import os
import pathlib
import struct
import sys

MAGIC = b'GMSTG016'
CAPACITY = 16 * 1024 * 1024 * 1024
OLD_MAGIC = b'GMSTG001'
OLD_CAPACITY = 4 * 1024 * 1024 * 1024
SECTOR = 512
MAX_FILES = 64
HEADER = struct.Struct('<8sQI')
ENTRY = struct.Struct('<48sQQ')
DATA_START = SECTOR + MAX_FILES * ENTRY.size


def align(n):
    return (n + SECTOR - 1) // SECTOR * SECTOR


def directory(f, expected_magic=MAGIC, expected_capacity=CAPACITY):
    f.seek(0)
    raw = f.read(SECTOR)
    if len(raw) != SECTOR:
        raise ValueError('truncated header')
    magic, capacity, count = HEADER.unpack_from(raw)
    if magic != expected_magic or capacity != expected_capacity or count > MAX_FILES:
        raise ValueError('invalid Gm-Stg header')
    if os.fstat(f.fileno()).st_size != expected_capacity:
        raise ValueError('incorrect image size')
    result = []
    for _ in range(count):
        raw = f.read(ENTRY.size)
        if len(raw) != ENTRY.size:
            raise ValueError('truncated directory')
        name, offset, size = ENTRY.unpack(raw)
        name = name.split(b'\0', 1)[0].decode('ascii')
        if not name or offset < DATA_START or offset % SECTOR or offset >= capacity or offset + size > capacity:
            raise ValueError('invalid directory entry')
        result.append((name, offset, size))
    extents = sorted((offset, offset + size) for _, offset, size in result if size)
    if any(end > next_start for (_, end), (next_start, _) in zip(extents, extents[1:])):
        raise ValueError('overlapping files')
    if len({name for name, _, _ in result}) != len(result):
        raise ValueError('duplicate file names')
    return result


def create(path):
    with open(path, 'xb') as f:
        f.write(HEADER.pack(MAGIC, CAPACITY, 0).ljust(DATA_START, b'\0'))
        f.truncate(CAPACITY)  # sparse on filesystems that support it


def put(image, source, name):
    encoded = name.encode('ascii')
    if not encoded or len(encoded) > 47 or any(c < 33 or c > 126 for c in encoded) or '/' in name or '\\' in name:
        raise ValueError('name must be 1-47 printable ASCII characters without spaces or slashes')
    with open(image, 'r+b') as f, open(source, 'rb') as src:
        entries = directory(f)
        if len(entries) == MAX_FILES or any(e[0] == name for e in entries):
            raise ValueError('directory full or name already exists')
        size = os.fstat(src.fileno()).st_size
        offset = align(max((off + length for _, off, length in entries), default=DATA_START))
        if size > 0xffffffff or offset >= CAPACITY or offset + size > CAPACITY:
            raise ValueError('file does not fit')
        f.seek(offset)
        remaining = size
        while remaining:
            chunk = src.read(min(1024 * 1024, remaining))
            if not chunk:
                raise ValueError('source changed while reading')
            f.write(chunk)
            remaining -= len(chunk)
        f.flush()
        f.seek(SECTOR + len(entries) * ENTRY.size)
        f.write(ENTRY.pack(encoded, offset, size))
        f.flush()
        f.seek(16)
        f.write(struct.pack('<I', len(entries) + 1))



def migrate(old_path, new_path):
    """Copy named file contents from a 4 GiB v1 image to a new 16 GiB image."""
    with open(old_path, 'rb') as old:
        entries = directory(old, OLD_MAGIC, OLD_CAPACITY)
        create(new_path)  # exclusive: never overwrite the source or an existing disk
        with open(new_path, 'r+b') as new:
            next_offset = DATA_START
            for index, (name, off, size) in enumerate(entries):
                if size > 0xffffffff:
                    raise ValueError('old file exceeds supported size')
                next_offset = align(next_offset)
                old.seek(off); new.seek(next_offset)
                remaining = size
                while remaining:
                    chunk = old.read(min(1024 * 1024, remaining))
                    if not chunk: raise ValueError('truncated source disk')
                    new.write(chunk); remaining -= len(chunk)
                new.seek(SECTOR + index * ENTRY.size)
                new.write(ENTRY.pack(name.encode('ascii'), next_offset, size))
                next_offset += size
            new.seek(16); new.write(struct.pack('<I', len(entries)))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    p = sub.add_parser('create'); p.add_argument('image')
    p = sub.add_parser('put'); p.add_argument('image'); p.add_argument('file'); p.add_argument('--name')
    p = sub.add_parser('list'); p.add_argument('image')
    p = sub.add_parser('migrate'); p.add_argument('old_image'); p.add_argument('new_image')
    args = parser.parse_args()
    try:
        if args.command == 'create': create(args.image)
        elif args.command == 'put': put(args.image, args.file, args.name or pathlib.Path(args.file).name)
        elif args.command == 'migrate': migrate(args.old_image, args.new_image)
        else:
            with open(args.image, 'rb') as f:
                for name, offset, size in directory(f): print(f'{name}\t{size} bytes\t@ {offset}')
    except (OSError, ValueError, UnicodeError) as exc:
        print(f'gmstg: {exc}', file=sys.stderr)
        return 1
    return 0

if __name__ == '__main__':
    sys.exit(main())
