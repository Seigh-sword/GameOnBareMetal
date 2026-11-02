#ifndef CXG_API_H
#define CXG_API_H
#include <stdint.h>
/* x86 32-bit cdecl ABI v1. Executable entry is the first byte of the CXG.
   Code returns to the hub; it runs unsandboxed with kernel privileges. */
typedef struct {
    uint32_t version;
    void (*clear)(uint8_t color);
    void (*put)(int x, int y, char character, uint8_t color);
    void (*write)(int x, int y, const char *text, uint8_t color);
    uint8_t (*keyboard_poll)(void);
} cxg_api;
typedef void (*cxg_entry)(const cxg_api *api);
#endif
