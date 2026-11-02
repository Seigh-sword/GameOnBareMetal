/* 1.44 MB floppy boot sector. Load 64 sectors at physical 0x10000. */
.code16
.section .text
.global _start
_start:
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %ss
    movw $0x7c00, %sp
    sti
    movb %dl, drive
    movw $0x0003, %ax
    int $0x10                   /* Standard 80x25 VGA text mode. */
    movw $0x1000, %ax
    movw %ax, %es
    xorw %bx, %bx
    movb $0, cylinder
    movb $0, head
    movb $2, sector
    movw $64, %si
read_next:
    movb $0x02, %ah
    movb $1, %al
    movb cylinder, %ch
    movb sector, %cl
    movb head, %dh
    movb drive, %dl
    int $0x13
    jc disk_error
    addw $512, %bx
    incb sector
    cmpb $19, sector
    jb next
    movb $1, sector
    xorb $1, head
    jnz next
    incb cylinder
next:
    decw %si
    jnz read_next
    cli
    lgdt gdt_descriptor
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0
    ljmpl $0x08, $protected
 disk_error:
    movw $error_message, %si
print_error:
    lodsb
    testb %al, %al
    jz halt
    movb $0x0e, %ah
    int $0x10
    jmp print_error
halt:
    cli
    hlt
    jmp halt
 drive: .byte 0
 cylinder: .byte 0
 head: .byte 0
 sector: .byte 0
 error_message: .asciz "Disk read error"
.align 8
gdt:
    .quad 0
    .quad 0x00cf9a000000ffff
    .quad 0x00cf92000000ffff
gdt_end:
gdt_descriptor:
    .word gdt_end - gdt - 1
    .long gdt
.code32
protected:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw %ax, %fs
    movw %ax, %gs
    movl $0x90000, %esp
    movl $0x10000, %eax
    jmp *%eax
.org 510
.word 0xaa55
