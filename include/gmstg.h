#ifndef GMSTG_H
#define GMSTG_H
#include <stdint.h>
/* Gm-Stg16 v1 on a primary IDE ATA disk, master=0 or slave=1.
   Returns 0 on success, -1 for invalid disk/IO or missing file.
   Read-only; names are case-sensitive ASCII. */
typedef struct { uint32_t first_sector; uint32_t size; } gmstg_file;
int gmstg_find(int drive, const char *name, gmstg_file *file);
/* Read one sector of a file into a 512-byte buffer. The final sector may
   contain padding; use file.size to determine how many bytes are meaningful. */
int gmstg_read_sector(int drive, const gmstg_file *file, uint32_t sector, void *buffer);
#endif
