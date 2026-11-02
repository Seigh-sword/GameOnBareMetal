import importlib.util
import pathlib
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
    assert disk.stat().st_size == 4096 * 1024 * 1024
    g.put(disk, source, 'sample.wad')
    with disk.open('rb') as f:
        entries = g.directory(f)
        assert entries == [('sample.wad', 4608, 800)]
        f.seek(entries[0][1])
        assert f.read(800) == source.read_bytes()
    try:
        g.put(disk, source, 'sample.wad')
        raise AssertionError('duplicate accepted')
    except ValueError:
        pass
    with disk.open('r+b') as f:
        f.seek(0); f.write(b'broken!!')
    with disk.open('rb') as f:
        try:
            g.directory(f)
            raise AssertionError('corrupt header accepted')
        except ValueError:
            pass
print('GmStg format OK')
