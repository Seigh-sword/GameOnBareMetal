#include "cxg.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned char *package;
static size_t package_len;
int gmstg_read_sector(int drive, const gmstg_file *file, uint32_t sector, void *buffer) {
    (void)drive; (void)file;
    size_t pos=(size_t)sector*512;
    if (pos>=package_len) return -1;
    size_t n=package_len-pos; if (n>512) n=512;
    memset(buffer,0,512); memcpy(buffer,package+pos,n); return 0;
}
static unsigned char *read_file(const char *path, size_t *size) {
    FILE *f=fopen(path,"rb"); assert(f);
    assert(fseek(f,0,SEEK_END)==0); long length=ftell(f); assert(length>=0);
    rewind(f); *size=(size_t)length;
    unsigned char *p=malloc(*size+1); assert(p && fread(p,1,*size,f)==*size); fclose(f); return p;
}
int main(void) {
    package=read_file("build/game.cxg.meta",&package_len);
    size_t original_len; unsigned char *original=read_file("build/sample_cxg.bin",&original_len);
    unsigned char *output=calloc(CXG_MAX_CODE,1); assert(output);
    gmstg_file f={0,(uint32_t)package_len};
    assert(cxg_load(0,&f,output,CXG_MAX_CODE)==0);
    assert(!memcmp(output,original,original_len));
    assert(cxg_load(0,&f,output,1)==-1);
    package[16+108] ^= 1; /* Corrupt executable CRC. */
    assert(cxg_load(0,&f,output,CXG_MAX_CODE)==-1);
    free(output); free(original); free(package);
    puts("CXG kernel loader OK");
}
