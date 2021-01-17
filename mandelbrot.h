#pragma once

#include "complex.h"
#include <cassert>

static int mandelbrot( const complex & c, double max_abs, int max_iter )
{
    complex z = c;
    int iter=0;
    while(z.abs() < max_abs && iter < max_iter-1) {
        z = z*z + c;
        iter++;
    }
    return iter;
}

static void set_pixel(unsigned char *image, int pos, unsigned int col_argb) 
{
    reinterpret_cast<unsigned int &>(image[pos*4]) =  col_argb;
}

static unsigned int make_color(int max_iter, int iter, int rank, int size) 
{
    assert(0 <= iter && iter < max_iter && 0 <= rank && rank < size); // check for invalid input

    union {
        unsigned int u32;
        unsigned char u8[4];
    } col_argb;

    constexpr unsigned char min = 8;
    constexpr unsigned char max = 248;

    unsigned char range = (max - min) / size;

    unsigned char r_min = min + ((rank * 11 + 7) % size) * range;
    unsigned char g_min = min + ((rank * 23 + 11) % size) * range;
    unsigned char b_min = min + ((rank * 37 + 19) % size) * range;

    float x = 1.f - float(iter) / max_iter;

    col_argb.u8[0] = x * range + b_min;
    col_argb.u8[1] = x * range + g_min;
    col_argb.u8[2] = x * range + r_min;
    col_argb.u8[3] = 255;

    return col_argb.u32;
}
