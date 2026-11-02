#ifndef CXG_H
#define CXG_H
#include <stdint.h>
#include "gmstg.h"
#define CXG_LOAD_ADDRESS 0x50000u
#define CXG_MAX_CODE (192u * 1024u)
/* Package reads are from GmStg; caller supplies storage for decoded code.
   Returns 0 only if the executable decodes and its CRC32 matches. */
int cxg_load(int drive, const gmstg_file *package, uint8_t *output, uint32_t capacity);
#endif
