#include "cxg.h"
#include "cxg_api.h"
#include <stdint.h>
static int saw_message;
static void clear(uint8_t c) { (void)c; }
static void put(int x,int y,char c,uint8_t color) { (void)x; (void)y; (void)c; (void)color; }
static void write_text(int x,int y,const char *s,uint8_t c) {
    (void)x; (void)y; (void)c;
    if (s[0]=='H' && s[1]=='e') saw_message=1;
}
static uint8_t keyboard(void) { return 0x1c; }
static void say(const char *s) { while (*s) __asm__ volatile ("outb %0, $0xe9" : : "a"((uint8_t)*s++)); }
void game_main(void) {
    gmstg_file file;
    cxg_api api={1,clear,put,write_text,keyboard};
    if (gmstg_find(0,"game.cxg.meta",&file) || cxg_load(0,&file,(uint8_t *)CXG_LOAD_ADDRESS,CXG_MAX_CODE)) {
        say("CXG FAIL\n");
    } else {
        ((cxg_entry)CXG_LOAD_ADDRESS)(&api);
        say(saw_message ? "CXG PASS\n" : "CXG FAIL\n");
    }
    for (;;) __asm__ volatile ("hlt");
}
