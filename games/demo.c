#include "game.h"

void game_main(void) {
    int x = 40, y = 12;
    screen_clear(0x01);
    screen_write(22, 3, "GAME ON BARE METAL", 0x0f);
    screen_write(15, 5, "A tiny game booted without an operating system.", 0x07);
    screen_write(20, 20, "Move @ with the arrow keys or WASD", 0x0e);
    screen_write(26, 22, "Made for your brother!", 0x0b);
    screen_put(x, y, '@', 0x0a);
    for (;;) {
        uint8_t key = keyboard_poll();
        int nx = x, ny = y;
        if (key == 0x4b || key == 0x1e) --nx;
        if (key == 0x4d || key == 0x20) ++nx;
        if (key == 0x48 || key == 0x11) --ny;
        if (key == 0x50 || key == 0x1f) ++ny;
        if (nx >= 0 && nx < SCREEN_WIDTH && ny >= 7 && ny < 19) {
            if (nx != x || ny != y) {
                screen_put(x, y, ' ', 0x01);
                x = nx; y = ny;
                screen_put(x, y, '@', 0x0a);
            }
        }
    }
}
