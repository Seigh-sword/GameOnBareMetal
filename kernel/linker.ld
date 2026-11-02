ENTRY(_start)
SECTIONS {
  . = 0x10000;
  .text : { *(.text.entry) *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  __bss_start = .;
  .bss : { *(.bss*) *(COMMON) }
  __bss_end = .;
  ASSERT(__bss_end <= 0x18000, "Kernel and game memory exceed 32 KiB")
  /DISCARD/ : { *(.eh_frame*) *(.comment*) *(.note*) }
}
