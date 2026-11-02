#include "game.h"
#include "gmstg.h"
#include "hub.h"

static void title(const char *page) {
    screen_clear(0x01);
    screen_write(3, 2, "GAME ON BARE METAL", 0x0f);
    screen_write(3, 3, "------------------", 0x09);
    screen_write(3, 5, page, 0x0b);
    screen_write(3, 23, "Arrow keys / W S: navigate   Enter: select   Esc: back", 0x07);
}
static void option(int y, const char *label, int selected) {
    screen_write(5, y, selected ? ">" : " ", 0x0e);
    screen_write(8, y, label, selected ? 0x0f : 0x07);
}
static void render(const hub_state *s) {
    if (s->page == HUB_HOME) {
        title("Game hub");
        option(8, "Play", s->selected == 0);
        option(10, "Settings", s->selected == 1);
        option(12, "Mode", s->selected == 2);
    } else if (s->page == HUB_PLAY) {
        title("Play");
        option(8, "Doom  [not installed]", 1);
        screen_write(8, 11, "Doom support is under development.", 0x07);
    } else if (s->page == HUB_SETTINGS) {
        title("Settings");
        screen_write(8, 8, "Display: 80 x 25 text (fixed)", 0x0f);
        screen_write(8, 10, "Drivers: built in; updates and repair not available yet", 0x07);
        screen_write(8, 12, "Resolution switching is not supported yet.", 0x07);
    } else if (s->page == HUB_MODE) {
        title("Mode");
        screen_write(8, 8, "Standard mode", 0x0f);
        screen_write(8, 10, "More modes are planned; no mode changes available yet.", 0x07);
    } else {
        gmstg_file f;
        int found = gmstg_find(0, "doom.wad", &f) == 0 || gmstg_find(0, "freedoom2.wad", &f) == 0;
        title("Doom");
        screen_write(8, 8, found ? "Game data found on GmStg primary IDE disk." : "No doom.wad or freedoom2.wad found on primary IDE disk.", 0x0f);
        screen_write(8, 10, "Doom engine is not installed and cannot be launched yet.", 0x0e);
        screen_write(8, 12, "Press Esc to return.", 0x07);
    }
}
void game_main(void) {
    hub_state state = { HUB_HOME, 0 };
    render(&state);
    for (;;) {
        uint8_t key = keyboard_poll();
        hub_action action;
        if (key == 0x48 || key == 0x11) action = HUB_UP;
        else if (key == 0x50 || key == 0x1f) action = HUB_DOWN;
        else if (key == 0x1c) action = HUB_ENTER;
        else if (key == 0x01) action = HUB_BACK;
        else continue;
        hub_step(&state, action);
        render(&state);
    }
}
