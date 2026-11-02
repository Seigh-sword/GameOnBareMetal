/* Stage 2: check conventional RAM, configure video, load stages 3 and 4. */
.code16
.section .text
.global _start
_start:
    movb %dl, drive
    int $0x12
    cmpw $604, %ax
    jb memory_error
.if VGA_GRAPHICS
    movw $0x0013, %ax
.else
    movw $0x0003, %ax
.endif
    int $0x10
    xorw %ax, %ax
    movw %ax, %es
    movw $0x8800, %bx
    movw $0x0204, %ax           /* Sectors 6-9, within first floppy track. */
    movw $0x0006, %cx
    xorw %dx, %dx
    movb drive, %dl
    int $0x13
    jc disk_error
    movb drive, %dl
    ljmp $0, $0x8800
memory_error:
    movw $memory_message, %si
    jmp error
disk_error:
    movw $disk_message, %si
error:
    lodsb
    testb %al, %al
    jz halt
    movb $0x0e, %ah
    int $0x10
    jmp error
halt:
    cli
    hlt
    jmp halt
drive: .byte 0
memory_message: .asciz "Insufficient conventional memory (604 KiB required)"
disk_message: .asciz "Stage 3/4 disk read error"
