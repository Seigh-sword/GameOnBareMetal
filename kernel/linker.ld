ENTRY(_start)
SECTIONS {
  . = 0x10000;
  .text : { *(.text.entry) *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  __bss_start = .;
  .bss : { *(.bss*) *(COMMON) }
  __bss_end = .;
  ASSERT(__bss_end <= 0x50000, "Kernel static memory overlaps CXG load area")
  /DISCARD/ : { *(.eh_frame*) *(.comment*) *(.note*) }
}
