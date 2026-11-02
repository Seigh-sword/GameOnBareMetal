#include <stdint.h>
/* Verify kernel loading beyond 64 KiB when the image is an IDE disk. */
static const volatile uint8_t probe[100000] = { [0] = 0x35, [99999] = 0xa7 };
static void say(const char *s) {
    while (*s) __asm__ volatile ("outb %0, $0xe9" : : "a"((uint8_t)*s++));
}
void game_main(void) {
    say(probe[0] == 0x35 && probe[99999] == 0xa7 ? "HDD PASS\n" : "HDD FAIL\n");
    for (;;) __asm__ volatile ("hlt");
}
