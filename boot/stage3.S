/* Stage 3: enable and verify the A20 line before entering protected mode. */
.code16
.section .text
.global _start
_start:
    movb %dl, drive
    inb $0x92, %al
    orb $2, %al
    andb $0xfe, %al              /* Do not assert fast reset. */
    outb %al, $0x92
    pushf
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw $0xffff, %ax
    movw %ax, %es
    movb 0x0500, %al
    movb %es:0x0510, %bl
    movb $0, 0x0500
    movb $0xff, %es:0x0510
    cmpb $0, 0x0500
    sete %cl
    movb %bl, %es:0x0510
    movb %al, 0x0500
    xorw %ax, %ax
    movw %ax, %es
    popf
    testb %cl, %cl
    jz a20_error
    movb drive, %dl
    ljmp $0, $0x8c00
a20_error:
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
message: .asciz "A20 could not be enabled"
