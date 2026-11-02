#ifndef GAME_H
#define GAME_H
#include <stdint.h>
/* VGA text mode: 80 columns, 25 rows. color = background << 4 | foreground. */
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
void screen_clear(uint8_t color);
void screen_put(int x, int y, char character, uint8_t color);
void screen_write(int x, int y, const char *text, uint8_t color);
/* Nonblocking; returns a raw PS/2 set 1 scancode, or 0 if no key is ready. */
uint8_t keyboard_poll(void);
/* Each game supplies this entry point. */
void game_main(void);
#endif
