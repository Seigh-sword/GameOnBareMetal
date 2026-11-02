#include "cxg_api.h"
/* Entry must be first in the raw image. No globals requiring BSS or libc. */
__attribute__((section(".text.entry")))
void cxg_main(const cxg_api *api) {
    if (!api || api->version != 1) return;
    api->clear(0x01);
    api->write(8, 8, "Hello from a compressed CXG game!", 0x0f);
    api->write(8, 11, "Press Enter to return to the game hub.", 0x0e);
    while (api->keyboard_poll() != 0x1c) { }
}
