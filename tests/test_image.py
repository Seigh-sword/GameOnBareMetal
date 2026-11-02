from pathlib import Path

image = Path('build/game.img').read_bytes()
boot = Path('build/boot.bin').read_bytes()
kernel = Path('build/kernel.bin').read_bytes()
assert len(image) == 1474560
assert image[:512] == boot and image[510:512] == b'\x55\xaa'
assert image[512:512 + len(kernel)] == kernel
assert len(kernel) <= 32768
print('Image layout OK')
