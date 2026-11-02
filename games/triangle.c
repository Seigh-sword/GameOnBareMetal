#include "graphics.h"
#include "compactgl.h"
void game_main(void) {
    graphics_init();
    cgl_surface surface = { (volatile uint8_t *)0xa0000, 320*200, 320, 200, 320 };
    const cgl_vertex vertices[3] = { {160,20}, {32,170}, {287,170} };
    cgl_clear(&surface, 1);
    cgl_triangle(&surface, vertices, 0xe4);
    for (;;) __asm__ volatile ("hlt");
}
