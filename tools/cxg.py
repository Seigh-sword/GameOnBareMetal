#!/usr/bin/env python3
"""CXG.META v1: indexed LZ-compressed game bundles (not ZIP)."""
import argparse
import binascii
from collections import defaultdict, deque
from pathlib import Path, PurePosixPath
import struct
import sys

MAGIC = b'CXGMETA1'
HEADER = struct.Struct('<8sII') # magic, count, reserved
ENTRY = struct.Struct('<96sIIII') # name, offset, packed size, size, crc32
MAX_FILES = 64
MAX_SIZE = 16 * 1024 * 1024
MAX_UNPACKED = 32 * 1024 * 1024
SECTIONS = ('_METADATA', '_HEADERS', '_LIBS', '_DEPS', '_ASSETS')
EXECUTABLE = '_HEADERS/game.cxg'


def compress(data):
    # Independent LZ blocks: 0..127 = literal length-1; 128..255 =
    # match length-3 followed by a little-endian 16-bit backward distance.
    table = defaultdict(lambda: deque(maxlen=32))
    output, literals = bytearray(), bytearray()
    def flush():
        if literals:
            output.append(len(literals) - 1); output.extend(literals); literals.clear()
    def remember(i):
        if i + 3 <= len(data): table[data[i:i+3]].append(i)
    i = 0
    while i < len(data):
        best_len, best_dist = 0, 0
        if i + 3 <= len(data):
            for prev in reversed(table.get(data[i:i+3], ())):
                distance = i - prev
                if distance > 65535: break
                n = 3
                while n < 130 and i+n < len(data) and data[prev+n] == data[i+n]: n += 1
                if n > best_len: best_len, best_dist = n, distance
                if n == 130: break
        if best_len >= 4:
            flush(); output.append(0x80 | (best_len-3)); output.extend(struct.pack('<H', best_dist))
            for j in range(i, i+best_len): remember(j)
            i += best_len
        else:
            literals.append(data[i]); remember(i); i += 1
            if len(literals) == 128: flush()
    flush()
    return bytes(output)


def decompress(data, expected):
    if expected > MAX_SIZE: raise ValueError('file too large')
    out = bytearray(); i = 0
    while i < len(data):
        token = data[i]; i += 1
        if token < 128:
            n = token+1
            if i+n > len(data) or len(out)+n > expected: raise ValueError('invalid literal')
            out.extend(data[i:i+n]); i += n
        else:
            n = (token & 127)+3
            if i+2 > len(data) or len(out)+n > expected: raise ValueError('invalid match')
            distance = data[i] | data[i+1] << 8; i += 2
            if not distance or distance > len(out): raise ValueError('invalid distance')
            for _ in range(n): out.append(out[-distance])
    if len(out) != expected: raise ValueError('incorrect uncompressed size')
    return bytes(out)


def valid_name(name):
    parts = name.split('/')
    return (len(name.encode('ascii')) <= 95 and len(parts) >= 2 and parts[0] in SECTIONS
            and all(p not in ('', '.', '..') and all(33 <= ord(c) <= 126 and c not in '\\:' for c in p) for p in parts))


def pack(executable, folders, out):
    files = [(EXECUTABLE, Path(executable).read_bytes())]
    for section, folder in folders.items():
        if folder is None: continue
        root = Path(folder)
        if not root.is_dir(): raise ValueError(f'{folder} is not a directory')
        for item in sorted(root.rglob('*')):
            if item.is_symlink(): raise ValueError('symlinks are not allowed')
            if item.is_file(): files.append((section+'/'+item.relative_to(root).as_posix(), item.read_bytes()))
    if len(files) > MAX_FILES: raise ValueError('too many files')
    if sum(len(data) for _, data in files) > MAX_UNPACKED: raise ValueError('total uncompressed size too large')
    names = set(); records = []; blobs = []; offset = HEADER.size + ENTRY.size*len(files)
    for name, data in files:
        if not valid_name(name) or name in names: raise ValueError(f'invalid or duplicate name: {name}')
        if len(data) > MAX_SIZE: raise ValueError('file too large')
        names.add(name); blob = compress(data)
        records.append(ENTRY.pack(name.encode(), offset, len(blob), len(data), binascii.crc32(data)))
        blobs.append(blob); offset += len(blob)
    if offset > MAX_SIZE: raise ValueError('package too large')
    Path(out).write_bytes(HEADER.pack(MAGIC,len(files),0) + b''.join(records) + b''.join(blobs))


def read_package(path):
    data = Path(path).read_bytes()
    if len(data) > MAX_SIZE or len(data) < HEADER.size: raise ValueError('invalid package size')
    magic, count, reserved = HEADER.unpack_from(data)
    if magic != MAGIC or not 1 <= count <= MAX_FILES or reserved: raise ValueError('invalid header')
    end = HEADER.size + count*ENTRY.size
    if end > len(data): raise ValueError('truncated directory')
    result, ranges, names, total = [], [], set(), 0
    for i in range(count):
        raw, off, packed, size, crc = ENTRY.unpack_from(data, HEADER.size+i*ENTRY.size)
        if b'\0' not in raw or any(raw[raw.index(b'\0'):]): raise ValueError('invalid name padding')
        name = raw.split(b'\0',1)[0].decode('ascii')
        if not valid_name(name) or name in names or off < end or off+packed > len(data): raise ValueError('invalid record')
        total += size
        if total > MAX_UNPACKED: raise ValueError('total uncompressed size too large')
        names.add(name); ranges.append((off,off+packed))
        decoded = decompress(data[off:off+packed], size)
        if binascii.crc32(decoded) != crc: raise ValueError('checksum mismatch')
        result.append((name, decoded))
    ranges.sort()
    if any(a[1] > b[0] for a,b in zip(ranges,ranges[1:])) or EXECUTABLE not in names: raise ValueError('overlap or missing executable')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    p = sub.add_parser('pack'); p.add_argument('--executable', required=True); p.add_argument('--output', required=True)
    for section in ('metadata', 'headers', 'libs', 'deps', 'assets'): p.add_argument('--'+section)
    p = sub.add_parser('list'); p.add_argument('package')
    p = sub.add_parser('extract'); p.add_argument('package'); p.add_argument('destination')
    args = parser.parse_args()
    try:
        if args.command == 'pack':
            pack(args.executable, {s:getattr(args,s[1:].lower()) for s in SECTIONS}, args.output)
        else:
            files = read_package(args.package)
            if args.command == 'list':
                for name, content in files: print(f'{name}\t{len(content)} bytes')
            else:
                root = Path(args.destination); root.mkdir(parents=True, exist_ok=False)
                for name, content in files:
                    target = root.joinpath(*PurePosixPath(name).parts)
                    target.parent.mkdir(parents=True, exist_ok=True)
                    target.write_bytes(content)
    except (OSError, ValueError, UnicodeError, struct.error) as exc:
        print(f'cxg: {exc}', file=sys.stderr); return 1
    return 0

if __name__ == '__main__': sys.exit(main())
