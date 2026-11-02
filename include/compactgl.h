#ifndef COMPACTGL_H
#define COMPACTGL_H
#include <stddef.h>
#include <stdint.h>
/* Experimental explicit software renderer; NOT OpenGL ES or Vulkan compatible.
   8-bit 3:3:2 RGB, top-left origin, integer pixel coordinates. */
typedef struct {
    volatile uint8_t *pixels;
    size_t length;
    unsigned width, height, stride;
} cgl_surface;
typedef struct { int x, y; } cgl_vertex;
/* Returns -1 for invalid inputs, 0 on success. No hidden global state.
   The caller owns the surface and must synchronize overlapping access. */
int cgl_clear(const cgl_surface *surface, uint8_t color);
int cgl_triangle(const cgl_surface *surface, const cgl_vertex vertices[3], uint8_t color);
#endif
