/************************************************/
/* Image.h, (c) Rene Puchinger                  */
/*                                              */
/* class for PGM / PPM image handling           */
/************************************************/

#ifndef IMAGE_H
#define	IMAGE_H

#include <stddef.h>

#define HD_WIDTH 1920
#define HD_HEIGHT 1080

class Image {
    /* we use the YCbCr color model; for grayscale images use only the Y component */
    float* Y;
    float* Cb;
    float* Cr;
    int size_x;
    int size_y;
    int extra_x;
    int extra_y;
    void symmetrize(int component);
public:
    Image();
    Image(bool is_color, int _size_x, int _size_y, int _extra_x, int _extra_y);
    ~Image();
    int get_size() { return (size_x + extra_x) * (size_y + extra_y); }
    int get_size_x() { return size_x; }
    int get_size_y() { return size_y; }
    int get_extra_x() { return extra_x; }
    int get_extra_y() { return extra_y; }
    bool is_color() { return (Cb != NULL); }
    float& at(int component, int x, int y);  /* component = 0 for Y, 1 for Cb, 2 for Cr */
    void load(char* fname, int* num_stages);
    void load_v210(char* fname, int* num_stages);
    void save(char* fname);
};

#endif
