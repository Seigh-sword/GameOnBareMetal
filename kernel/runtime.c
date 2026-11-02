#include "game.h"

static volatile uint16_t *const vga = (volatile uint16_t *)0xb8000;
void screen_put(int x, int y, char character, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        vga[y * SCREEN_WIDTH + x] = (uint16_t)(uint8_t)character | ((uint16_t)color << 8);
}
void screen_clear(uint8_t color) {
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
        for (int x = 0; x < SCREEN_WIDTH; ++x) screen_put(x, y, ' ', color);
}
void screen_write(int x, int y, const char *text, uint8_t color) {
    while (*text && x < SCREEN_WIDTH) screen_put(x++, y, *text++, color);
}
static uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
uint8_t keyboard_poll(void) {
    return (inb(0x64) & 1) ? inb(0x60) : 0;
}
