from pathlib import Path

image = Path('build/game.img').read_bytes()
boot = Path('build/boot.bin').read_bytes()
stage2 = Path('build/stage2.bin').read_bytes()
kernel = Path('build/kernel.bin').read_bytes()
assert len(image) == 1474560
assert image[:512] == boot and image[510:512] == b'\x55\xaa'
assert len(stage2) <= 2048 and image[512:512 + len(stage2)] == stage2
assert image[512 + 2048:512 + 2048 + len(kernel)] == kernel
assert len(kernel) <= 512 * 1024
print('Image layout OK')
