from pathlib import Path

image = Path('build/game.img').read_bytes()
boot = Path('build/boot.bin').read_bytes()
kernel = Path('build/kernel.bin').read_bytes()
assert len(image) == 1474560
assert image[:512] == boot and image[510:512] == b'\x55\xaa'
offset = 512
for name, size in [('stage2', 2048), ('stage3', 1024), ('stage4', 1024)]:
    stage = Path(f'build/{name}.bin').read_bytes()
    assert 0 < len(stage) <= size and image[offset:offset+len(stage)] == stage
    assert not any(image[offset+len(stage):offset+size])
    offset += size
assert image[offset:offset+len(kernel)] == kernel
assert len(kernel) <= 512 * 1024
print('Four-stage image layout OK')
