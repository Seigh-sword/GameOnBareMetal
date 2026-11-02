#include "gmstg.h"
#include <stddef.h>
#define ATA_DATA 0x1f0
#define ATA_STATUS 0x1f7
#define TIMEOUT 1000000
static uint8_t in8(uint16_t p) { uint8_t x; __asm__ volatile("inb %1,%0":"=a"(x):"Nd"(p)); return x; }
static uint16_t in16(uint16_t p) { uint16_t x; __asm__ volatile("inw %1,%0":"=a"(x):"Nd"(p)); return x; }
static void out8(uint16_t p, uint8_t x) { __asm__ volatile("outb %0,%1"::"a"(x),"Nd"(p)); }
static int read_sector(int drive, uint32_t lba, uint8_t *buffer) {
    if ((drive != 0 && drive != 1) || lba >= 0x10000000) return -1;
    out8(0x1f6, (uint8_t)(0xe0 | (drive << 4) | (lba >> 24)));
    /* ATA drive select requires at least 400 ns before accessing registers. */
    for (int i=0;i<4;i++) (void)in8(0x3f6);
    for (int i=0;i<TIMEOUT;i++) {
        uint8_t s=in8(ATA_STATUS);
        if (s==0 || s==0xff) return -1;
        if (!(s&0x80)) break;
        if (i==TIMEOUT-1) return -1;
    }
    out8(0x1f2,1); out8(0x1f3,(uint8_t)lba); out8(0x1f4,(uint8_t)(lba>>8));
    out8(0x1f5,(uint8_t)(lba>>16)); out8(0x1f7,0x20);
    for (int i=0;i<TIMEOUT;i++) {
        uint8_t s=in8(ATA_STATUS);
        if (s==0 || s==0xff) return -1;
        if (!(s&0x80) && (s&0x21)) return -1;
        if (!(s&0x80) && (s&0x08)) {
            for (int j=0;j<256;j++) { uint16_t v=in16(ATA_DATA); buffer[j*2]=(uint8_t)v; buffer[j*2+1]=(uint8_t)(v>>8); }
            return 0;
        }
    }
    return -1;
}
#ifdef USE_RUST
extern uint32_t gmstg_rust_u32(uint8_t, uint8_t, uint8_t, uint8_t);
#endif
static uint32_t u32(const uint8_t *p) {
#ifdef USE_RUST
    return gmstg_rust_u32(p[0], p[1], p[2], p[3]);
#else
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
#endif
}
int gmstg_find(int drive, const char *name, gmstg_file *file) {
    uint8_t sector[512];
    const char magic[]="GMSTG001";
    if (!name || !file || read_sector(drive,0,sector)) return -1;
    for (int i=0;i<8;i++) if (sector[i]!=(uint8_t)magic[i]) return -1;
    /* Exactly 4 GiB, encoded as a 64-bit little endian number. */
    if (u32(sector+8)!=0 || u32(sector+12)!=1) return -1;
    uint32_t count=u32(sector+16);
    if (count>64) return -1;
    for (uint32_t i=0;i<count;i++) {
        if ((i%8)==0 && read_sector(drive,1+i/8,sector)) return -1;
        const uint8_t *e=sector+(i%8)*64;
        int j=0;
        while (j<48 && name[j] && (uint8_t)name[j]==e[j]) j++;
        if (j==48 || name[j] || e[j]) continue;
        uint32_t off=u32(e+48), high=u32(e+52), size=u32(e+56), size_hi=u32(e+60);
        if (high || size_hi || off<4608 || (off&511) || (uint64_t)off + size > 0x100000000ull) return -1;
        file->first_sector=off>>9; file->size=size; return 0;
    }
    return -1;
}
int gmstg_read_sector(int drive, const gmstg_file *file, uint32_t sector, void *buffer) {
    if (!file || !buffer || sector >= (file->size >> 9) + ((file->size & 511) != 0)) return -1;
    if (file->first_sector > 0x7fffff || sector > 0x7fffff-file->first_sector) return -1;
    return read_sector(drive,file->first_sector+sector,(uint8_t *)buffer);
}
