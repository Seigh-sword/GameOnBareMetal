"""Boot the raw image as an IDE hard disk, matching QEMU's default -drive."""
import subprocess
import sys

command = ['qemu-system-x86_64', '-drive', f'format=raw,file={sys.argv[1]}',
           '-m', '64', '-display', 'none', '-monitor', 'none', '-serial', 'none',
           '-debugcon', 'stdio', '-no-reboot']
marker = (sys.argv[2] if len(sys.argv) > 2 else 'HDD').encode('ascii')
if len(sys.argv) > 3:
    command += ['-drive', f'format=raw,file={sys.argv[3]},if=ide,index=1']
try:
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=8)
    output, stderr = result.stdout, result.stderr
except subprocess.TimeoutExpired as exc:
    output, stderr = exc.stdout or b'', exc.stderr or b''
if marker + b' PASS' not in output or marker + b' FAIL' in output:
    raise SystemExit(f'Hard disk boot test failed: {output!r}; {stderr!r}')
print('QEMU default IDE hard disk boot OK')
