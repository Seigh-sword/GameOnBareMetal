import importlib.util
import pathlib
import struct
import tempfile

spec = importlib.util.spec_from_file_location('gmstg', 'tools/gmstg.py')
g = importlib.util.module_from_spec(spec)
spec.loader.exec_module(g)
with tempfile.TemporaryDirectory() as tmp:
    root = pathlib.Path(tmp)
    disk = root / 'storage.img'
    source = root / 'sample.wad'
    source.write_bytes(b'WAD!' * 200)
    g.create(disk)
    assert disk.stat().st_size == 16 * 1024 ** 3
    g.put(disk, source, 'sample.wad')
    with disk.open('rb') as f:
        entries = g.directory(f)
        assert entries == [('sample.wad', 4608, 800)]
        f.seek(entries[0][1]); assert f.read(800) == source.read_bytes()
    try:
        g.put(disk, source, 'sample.wad')
        raise AssertionError('duplicate accepted')
    except ValueError: pass
    # Create a legacy 4 GiB disk and verify migration of real file contents.
    old = root / 'old.img'; migrated = root / 'migrated.img'
    with old.open('wb') as f:
        f.write(g.HEADER.pack(g.OLD_MAGIC,g.OLD_CAPACITY,1).ljust(512,b'\0'))
        f.write(g.ENTRY.pack(b'old.wad',4608,800).ljust(4096,b'\0'))
        f.write(source.read_bytes()); f.truncate(g.OLD_CAPACITY)
    g.migrate(old,migrated)
    with migrated.open('rb') as f:
        assert g.directory(f) == [('old.wad',4608,800)]
        f.seek(4608); assert f.read(800) == source.read_bytes()
    # Verify allocation beyond the previous 4 GiB limit.
    high = 12 * 1024 ** 3
    with disk.open('r+b') as f:
        f.seek(512+g.ENTRY.size); f.write(g.ENTRY.pack(b'high.wad',high,4))
        f.seek(16); f.write(struct.pack('<I',2))
        f.seek(high); f.write(b'HIGH')
    with disk.open('rb') as f:
        assert g.directory(f)[1] == ('high.wad',high,4)
    g.put(disk, source, 'later.wad')
    with disk.open('rb') as f:
        assert g.directory(f)[2][1] > high
    with disk.open('r+b') as f:
        f.seek(0); f.write(b'broken!!')
    with disk.open('rb') as f:
        try:
            g.directory(f)
            raise AssertionError('corrupt header accepted')
        except ValueError: pass
print('Gm-Stg16 format and legacy migration OK')
