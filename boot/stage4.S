/* Stage 4: load the kernel from floppy and enter protected mode. */
.code16
.section .text
.global _start
_start:
    movb %dl, drive
    movw $0x1000, %ax
    movw %ax, %es
    movb $0, cylinder
    movb $0, head
    movb $10, sector
    movw $1024, remaining
read_next:
    xorw %bx, %bx
    movw $0x0201, %ax
    movb cylinder, %ch
    movb sector, %cl
    movb head, %dh
    movb drive, %dl
    int $0x13
    jc disk_error
    movw %es, %ax
    addw $0x20, %ax              /* Advance 512 bytes without segment wrap. */
    movw %ax, %es
    incb sector
    cmpb $19, sector
    jb next
    movb $1, sector
    xorb $1, head
    jnz next
    incb cylinder
next:
    decw remaining
    jnz read_next
    cli
    lgdt gdt_descriptor
    movl %cr0, %eax
    andl $0xfffffff3, %eax
    orl $3, %eax
    movl %eax, %cr0
    ljmpl $0x08, $protected
disk_error:
    movw $disk_message, %si
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
remaining: .word 0
disk_message: .asciz "Kernel disk read error"
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
    movl $0x97000, %esp
    movl $0x10000, %eax
    jmp *%eax
