#include "cxg.h"
#define HEADER 16u
#define RECORD 112u
#define LIMIT (16u * 1024u * 1024u)
typedef struct {
    int drive;
    const gmstg_file *file;
    uint32_t sector;
    uint8_t data[512];
} reader;
static int byte_at(reader *r, uint32_t pos, uint8_t *value) {
    if (pos >= r->file->size) return -1;
    uint32_t sector = pos >> 9;
    if (sector != r->sector) {
        if (gmstg_read_sector(r->drive, r->file, sector, r->data)) return -1;
        r->sector = sector;
    }
    *value = r->data[pos & 511];
    return 0;
}
static int word_at(reader *r, uint32_t p, uint32_t *out) {
    uint8_t b[4];
    for (unsigned i=0;i<4;i++) if (byte_at(r,p+i,&b[i])) return -1;
    *out = (uint32_t)b[0] | (uint32_t)b[1]<<8 | (uint32_t)b[2]<<16 | (uint32_t)b[3]<<24;
    return 0;
}
static uint32_t crc32(const uint8_t *p, uint32_t len) {
    uint32_t crc = 0xffffffffu;
    for (uint32_t i=0;i<len;i++) {
        crc ^= p[i];
        for (int bit=0;bit<8;bit++) crc = (crc>>1) ^ (0xedb88320u & (0u-(crc&1u)));
    }
    return ~crc;
}
int cxg_load(int drive, const gmstg_file *file, uint8_t *out, uint32_t capacity) {
    const char magic[] = "CXGMETA1", wanted[] = "_HEADERS/game.cxg";
    if (!file || !out || file->size < HEADER || file->size > LIMIT) return -1;
    reader r = { .drive=drive, .file=file, .sector=0xffffffffu };
    uint8_t b;
    for (unsigned i=0;i<8;i++) if (byte_at(&r,i,&b) || b!=(uint8_t)magic[i]) return -1;
    uint32_t count, reserved;
    if (word_at(&r,8,&count) || word_at(&r,12,&reserved) || reserved || !count || count>64 || HEADER+count*RECORD>file->size) return -1;
    uint32_t start=0, packed=0, size=0, expected=0;
    int found=0;
    for (uint32_t i=0;i<count;i++) {
        uint32_t p=HEADER+i*RECORD, off, compressed, length, crc;
        int match=1;
        for (unsigned j=0;j<96;j++) {
            if (byte_at(&r,p+j,&b)) return -1;
            uint8_t expected_char = j < sizeof(wanted) ? (uint8_t)wanted[j] : 0;
            if (b != expected_char) match=0;
        }
        if (word_at(&r,p+96,&off) || word_at(&r,p+100,&compressed) || word_at(&r,p+104,&length) || word_at(&r,p+108,&crc)) return -1;
        if (off < HEADER+count*RECORD || off > file->size || compressed > file->size-off || length>LIMIT) return -1;
        if (match) {
            if (found || !length || length>capacity || length>CXG_MAX_CODE) return -1;
            found=1; start=off; packed=compressed; size=length; expected=crc;
        }
    }
    if (!found) return -1;
    uint32_t pos=start, end=start+packed, written=0;
    while (pos<end) {
        uint8_t token;
        if (byte_at(&r,pos++,&token)) return -1;
        if (token<128) {
            uint32_t n=(uint32_t)token+1;
            if (n>end-pos || n>size-written) return -1;
            while (n--) if (byte_at(&r,pos++,&out[written++])) return -1;
        } else {
            uint32_t n=(token&127u)+3u;
            uint8_t lo,hi;
            if (end-pos<2 || n>size-written || byte_at(&r,pos++,&lo) || byte_at(&r,pos++,&hi)) return -1;
            uint32_t distance=(uint32_t)lo | (uint32_t)hi<<8;
            if (!distance || distance>written) return -1;
            while (n--) { out[written]=out[written-distance]; written++; }
        }
    }
    return written==size && crc32(out,size)==expected ? 0 : -1;
}
