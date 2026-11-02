"""QEMU integration smoke test for floppy/ISO boot and IDE storage reads."""
import subprocess
import sys

boot = sys.argv[1]
marker = (sys.argv[2] if len(sys.argv) > 2 else 'STORAGE').encode('ascii')
if boot.endswith('.iso'):
    media = ['-cdrom', boot, '-boot', 'd']
else:
    media = ['-drive', f'file={boot},format=raw,if=floppy', '-boot', 'a']
command = ['qemu-system-i386', '-machine', 'pc', '-m', '32M', '-display', 'none',
           '-monitor', 'none', '-serial', 'none', '-debugcon', 'stdio', '-no-reboot',
           '-drive', 'file=build/storage-test.img,format=raw,if=ide,index=0', *media]
try:
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=8)
    output = result.stdout
except subprocess.TimeoutExpired as exc:
    output = exc.stdout or b''
if marker + b' PASS' not in output or marker + b' FAIL' in output:
    raise SystemExit(f'Boot/storage test failed: {output!r}')
print(f'{boot}: boot and storage driver OK')
