.code32
.section .text.entry,"ax"
.global _start
.extern game_main
.extern __bss_start
.extern __bss_end
_start:
    cld
    fninit
    xorl %eax, %eax
    movl $__bss_start, %edi
    movl $__bss_end, %ecx
    subl %edi, %ecx
    rep stosb
    call game_main
1:  cli
    hlt
    jmp 1b
.section .note.GNU-stack,"",@progbits
