#include "compactgl.h"
#include <assert.h>
#include <stdio.h>
static unsigned char buffer[320*200 + 2];
int main(void) {
    cgl_surface s = { buffer + 1, 320*200, 320, 200, 320 };
    cgl_vertex tri[3] = { {32, 180}, {288, 180}, {160, 20} };
    assert(cgl_clear(&s, 3) == 0);
    assert(buffer[1] == 3 && buffer[sizeof(buffer)-2] == 3);
    assert(cgl_triangle(&s, tri, 224) == 0);
    assert(buffer[1+100*320+160] == 224 && buffer[1] == 3);
    assert(buffer[0] == 0 && buffer[sizeof(buffer)-1] == 0);
    tri[0].x = -1;
    assert(cgl_triangle(&s, tri, 1) == -1);
    s.length--;
    assert(cgl_clear(&s, 1) == -1);
    puts("CompactGL rasterizer OK");
}
