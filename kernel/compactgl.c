#include "compactgl.h"

static int valid(const cgl_surface *s) {
    if (!s || !s->pixels || !s->width || !s->height || s->width > 4096 ||
        s->height > 4096 || s->stride < s->width) return 0;
    /* Division avoids overflow when validating untrusted dimensions. */
    if ((size_t)s->stride > s->length / s->height) return 0;
    return 1;
}
int cgl_clear(const cgl_surface *s, uint8_t color) {
    if (!valid(s)) return -1;
    for (unsigned y=0; y<s->height; ++y)
        for (unsigned x=0; x<s->width; ++x) s->pixels[(size_t)y*s->stride+x]=color;
    return 0;
}
static int edge(cgl_vertex a, cgl_vertex b, int x, int y) {
    return (x-a.x)*(b.y-a.y) - (y-a.y)*(b.x-a.x);
}
int cgl_triangle(const cgl_surface *s, const cgl_vertex v[3], uint8_t color) {
    if (!valid(s) || !v) return -1;
    for (int i=0;i<3;i++)
        if (v[i].x<0 || v[i].y<0 || (unsigned)v[i].x>=s->width || (unsigned)v[i].y>=s->height) return -1;
    if (!edge(v[0],v[1],v[2].x,v[2].y)) return 0;
    int minx=v[0].x, maxx=minx, miny=v[0].y, maxy=miny;
    for (int i=1;i<3;i++) {
        if (v[i].x<minx) minx=v[i].x;
        if (v[i].x>maxx) maxx=v[i].x;
        if (v[i].y<miny) miny=v[i].y;
        if (v[i].y>maxy) maxy=v[i].y;
    }
    for (int y=miny;y<=maxy;y++) for (int x=minx;x<=maxx;x++) {
        int a=edge(v[0],v[1],x,y), b=edge(v[1],v[2],x,y), c=edge(v[2],v[0],x,y);
        if ((a>=0 && b>=0 && c>=0) || (a<=0 && b<=0 && c<=0))
            s->pixels[(size_t)y*s->stride+(unsigned)x]=color;
    }
    return 0;
}

/* Explicit nearest-neighbor texture transfer with bounds validation. */
int cgl_blit(const cgl_surface *s, const cgl_texture *t,
             unsigned x, unsigned y, unsigned width, unsigned height) {
    if (!valid(s) || !t || !t->pixels || !t->width || !t->height ||
        t->width > 4096 || t->height > 4096 || t->stride < t->width ||
        (size_t)t->stride > t->length / t->height || !width || !height ||
        x >= s->width || y >= s->height || width > s->width-x || height > s->height-y)
        return -1;
    for (unsigned row=0; row<height; ++row) {
        unsigned ty = row * t->height / height;
        for (unsigned col=0; col<width; ++col) {
            unsigned tx = col * t->width / width;
            s->pixels[(size_t)(y+row)*s->stride+x+col] = t->pixels[(size_t)ty*t->stride+tx];
        }
    }
    return 0;
}
