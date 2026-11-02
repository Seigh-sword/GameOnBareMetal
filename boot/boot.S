/* Stage 1: load the four-sector stage 2 from the boot floppy. */
.code16
.section .text
.global _start
_start:
    ljmp $0, $init
init:
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw $0x7c00, %sp
    sti
    movb %dl, drive
    movw $0x8000, %bx
    movw $0x0204, %ax
    movw $0x0002, %cx
    xorw %dx, %dx
    movb drive, %dl
    int $0x13
    jc fail
    movb drive, %dl
    ljmp $0, $0x8000
fail:
    movw $message, %si
1:  lodsb
    testb %al, %al
    jz 2f
    movb $0x0e, %ah
    int $0x10
    jmp 1b
2:  cli
    hlt
    jmp 2b
 drive: .byte 0
 message: .asciz "Stage 2 disk read error"
.org 510
.word 0xaa55
