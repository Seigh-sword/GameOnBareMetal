import importlib.util
from pathlib import Path
import random
import tempfile

spec = importlib.util.spec_from_file_location('cxg', 'tools/cxg.py')
c = importlib.util.module_from_spec(spec); spec.loader.exec_module(c)
rng = random.Random(12345)
for data in [b'', b'x', b'a'*5000, bytes(range(256))*80, rng.randbytes(8192), b'abcabcabc'*500]:
    assert c.decompress(c.compress(data),len(data)) == data
with tempfile.TemporaryDirectory() as temp:
    p=Path(temp); exe=p/'code.bin'; exe.write_bytes(b'CXG'*300)
    assets=p/'assets'; assets.mkdir(); (assets/'image.dat').write_bytes(b'hello'*1000)
    bundle=p/'game.cxg.meta'
    c.pack(exe, {'_ASSETS':assets}, bundle)
    files=dict(c.read_package(bundle))
    assert files[c.EXECUTABLE] == exe.read_bytes() and files['_ASSETS/image.dat'] == (assets/'image.dat').read_bytes()
    assert bundle.stat().st_size < len(exe.read_bytes()) + len((assets/'image.dat').read_bytes())
    original=bytearray(bundle.read_bytes()); original[-1] ^= 0x40; bundle.write_bytes(original)
    try:
        c.read_package(bundle)
        raise AssertionError('corrupt package accepted')
    except ValueError: pass
    assert not c.valid_name('_ASSETS/../evil')
    assert not c.valid_name('_ASSETS/evil\\path')
print('CXG compression and package validation OK')
