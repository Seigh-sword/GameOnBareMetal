ENTRY(cxg_main)
SECTIONS {
  . = 0x50000;
  .text : { *(.text.entry) *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(.bss*) *(COMMON) }
  ASSERT(SIZEOF(.bss) == 0, "CXG v1 games cannot have BSS")
  ASSERT(. <= 0x80000, "CXG exceeds 192 KiB")
  /DISCARD/ : { *(.eh_frame*) *(.comment*) *(.note*) }
}
