#include "game.h"
#include "gmstg.h"
#include "hub.h"
#include "cxg.h"
#include "cxg_api.h"

static int cxg_status;
static unsigned high_contrast;
static uint8_t text_color(uint8_t normal) {
    return high_contrast ? (normal == 0x0e ? 0x1f : 0x70) : normal;
}

static void title(const char *page) {
    screen_clear(high_contrast ? 0x70 : 0x01);
    screen_write(3, 2, "GAME ON BARE METAL", text_color(0x0f));
    screen_write(3, 3, "------------------", text_color(0x09));
    screen_write(3, 5, page, text_color(0x0b));
    screen_write(3, 23, "Arrow keys / W S: navigate   Enter: select   Esc: back", text_color(0x07));
}
static void option(int y, const char *label, int selected) {
    screen_write(5, y, selected ? ">" : " ", text_color(0x0e));
    screen_write(8, y, label, text_color(selected ? 0x0e : 0x07));
}
static void render(const hub_state *s) {
    if (s->page == HUB_HOME) {
        title("Game hub");
        option(8, "Play", s->selected == 0);
        option(10, "Settings", s->selected == 1);
        option(12, "Mode", s->selected == 2);
    } else if (s->page == HUB_PLAY) {
        title("Play");
        option(8, "Doom  [not installed]", s->selected == 0);
        option(10, "Launch game.cxg.meta", s->selected == 1);
        screen_write(8, 13, "Package must be on primary IDE Gm-Stg16 disk.", text_color(0x07));
    } else if (s->page == HUB_SETTINGS) {
        title("Settings");
        screen_write(8, 8, "Display: 80 x 25 text (fixed)", text_color(0x0f));
        screen_write(8, 10, "Drivers: built in; updates and repair not available yet", text_color(0x07));
        screen_write(8, 12, "Resolution switching is not supported yet.", text_color(0x07));
    } else if (s->page == HUB_MODE) {
        title("Mode");
        option(8, "Standard colors", s->selected == 0);
        option(10, "High contrast", s->selected == 1);
        screen_write(8, 13, "Enter applies the selected mode for this session.", text_color(0x07));
    } else if (s->page == HUB_CXG) {
        title("Compact Executable Game");
        screen_write(8, 8, cxg_status == 0 ? "Game returned to the hub." : "Package missing or invalid; game not started.", text_color(0x0f));
        screen_write(8, 10, "Press Esc to return.", text_color(0x07));
    } else {
        gmstg_file f;
        int found = gmstg_find(0, "doom.wad", &f) == 0 || gmstg_find(0, "freedoom2.wad", &f) == 0;
        title("Doom");
        screen_write(8, 8, found ? "Game data found on Gm-Stg16 primary IDE disk." : "No doom.wad or freedoom2.wad found on primary IDE disk.", text_color(0x0f));
        screen_write(8, 10, "Doom engine is not installed and cannot be launched yet.", text_color(0x0e));
        screen_write(8, 12, "Press Esc to return.", text_color(0x07));
    }
}
void game_main(void) {
    hub_state state = { HUB_HOME, 0, 0 };
    render(&state);
    for (;;) {
        uint8_t key = keyboard_poll();
        hub_action action;
        if (key == 0x48 || key == 0x11) action = HUB_UP;
        else if (key == 0x50 || key == 0x1f) action = HUB_DOWN;
        else if (key == 0x1c) action = HUB_ENTER;
        else if (key == 0x01) action = HUB_BACK;
        else continue;
        hub_page old_page = state.page;
        hub_step(&state, action);
        if (state.page == HUB_CXG && old_page != HUB_CXG) {
            gmstg_file file;
            cxg_status = -1;
            if (!gmstg_find(0, "game.cxg.meta", &file) &&
                !cxg_load(0, &file, (uint8_t *)CXG_LOAD_ADDRESS, CXG_MAX_CODE)) {
                cxg_api api = { 1, screen_clear, screen_put, screen_write, keyboard_poll };
                ((cxg_entry)CXG_LOAD_ADDRESS)(&api);
                cxg_status = 0;
            }
        }
        high_contrast = state.contrast;
        render(&state);
    }
}
