"""Add test data at 12 GiB to exercise the ATA driver beyond 32-bit offsets."""
import importlib.util
import pathlib
import struct
import sys
spec = importlib.util.spec_from_file_location('gmstg', 'tools/gmstg.py')
g = importlib.util.module_from_spec(spec)
spec.loader.exec_module(g)
path = pathlib.Path(sys.argv[1])
with path.open('r+b') as f:
    entries = g.directory(f)
    assert len(entries) < g.MAX_FILES
    offset = 12 * 1024 ** 3
    assert all(off + size <= offset for _, off, size in entries)
    f.seek(offset); f.write(b'HIGH')
    f.seek(g.SECTOR + len(entries)*g.ENTRY.size)
    f.write(g.ENTRY.pack(b'high.wad', offset, 4))
    f.seek(16); f.write(struct.pack('<I', len(entries)+1))
