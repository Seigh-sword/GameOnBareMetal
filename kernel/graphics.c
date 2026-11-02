#include "graphics.h"
#include <stdint.h>
static void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
void graphics_init(void) {
    outb(0x3c8, 0);
    for (int i=0; i<256; ++i) {
        outb(0x3c9, (uint8_t)(((i >> 5) & 7) * 63 / 7));
        outb(0x3c9, (uint8_t)(((i >> 2) & 7) * 63 / 7));
        outb(0x3c9, (uint8_t)((i & 3) * 63 / 3));
    }
}
