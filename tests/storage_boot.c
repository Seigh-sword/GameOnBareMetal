#include "gmstg.h"
#include <stdint.h>
/* Exercise reads across the old 32 KiB limit and 64 KiB boundaries. */
static const volatile uint8_t loader_probe[100000] = { [0] = 0x35, [99999] = 0xa7 };
static void say(const char *s) {
    while (*s) { __asm__ volatile ("outb %0, $0xe9" : : "a"((uint8_t)*s++)); }
}
void game_main(void) {
    gmstg_file f;
    uint8_t buf[512];
    if (loader_probe[0] != 0x35 || loader_probe[99999] != 0xa7 ||
        gmstg_find(0, "test.wad", &f) || f.size != 5 ||
        gmstg_read_sector(0, &f, 0, buf) || buf[0]!='H' || buf[4]!='O' ||
        gmstg_read_sector(0, &f, 1, buf) == 0 ||
        gmstg_find(0, "missing", &f) == 0) say("STORAGE FAIL\n");
    else say("STORAGE PASS\n");
    for (;;) __asm__ volatile ("hlt");
}
