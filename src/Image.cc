/************************************************/
/* Image.cc, (c) Rene Puchinger                 */
/************************************************/

#include <cstdio>
#include <cmath>
#include "Exception.h"
#include "Image.h"
#include "Encoder.h"

Image::Image() {
    size_x = 0;
    size_y = 0;
    extra_x = 0;
    extra_y = 0;
    Y = NULL;
    Cb = NULL;
    Cr = NULL;
}

Image::Image(bool is_color, int _size_x, int _size_y, int _extra_x, int _extra_y) {
    Y = NULL;
    Cb = NULL;
    Cr = NULL;
    size_x = _size_x;
    size_y = _size_y;
    extra_x = _extra_x;
    extra_y = _extra_y;
    Y = new float[(size_x+extra_x) * (size_y+extra_y)];
    if (Y == NULL)
        throw Exception(Exception::ERR_MEMORY);
    if (is_color) {
        Cb = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Cb == NULL)
            throw Exception(Exception::ERR_MEMORY);
        Cr = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Cr == NULL)
            throw Exception(Exception::ERR_MEMORY);
    }
}

Image::~Image() {
    if (Y)
        delete [] Y;
    if (Cb)
        delete [] Cb;
    if (Cr)
        delete [] Cr;

}

float& Image::at(int component, int x, int y) {
    if (component == 0)
        return Y[x + (y * (size_x + extra_x))];
    else if (component == 1)
        return Cb[x + (y * (size_x + extra_x))];
    else
        return Cr[x + (y * (size_x + extra_x))];
}

void Image::load(char* fname, int* num_stages) {
    FILE* fp = fopen(fname, "rb");
    if (fp == NULL)
        throw Exception(Exception::ERR_FILE_NOT_FOUND);
    if (getc(fp) != 'P')
        throw Exception(Exception::ERR_IMAGE_FORMAT);
    int type = getc(fp);
    if ((type != '2') && (type != '5') && (type != '3') && (type != '6'))
        throw Exception(Exception::ERR_IMAGE_FORMAT);
    while (getc(fp) != '\n');    /* skip whitespace */
    while (getc(fp) == '#') {    /* skip comments */
        while (getc(fp) != '\n');
    }
    fseek(fp, -1, SEEK_CUR);
    fscanf(fp, "%d", &size_x);
    fscanf(fp, "%d", &size_y);
    extra_x = 0;
    extra_y = 0;
	*num_stages = 4;
	int num_stages_x = 4;
	int num_stages_y = 4;
	while (size_x % (1 << (num_stages_x + 2)) == 0)
		num_stages_x++;
	while (size_y % (1 << (num_stages_y + 2)) == 0)
		num_stages_y++;
	*num_stages = (num_stages_x < num_stages_y) ? num_stages_x : num_stages_y;
    while ((size_x + extra_x) % (1 << (*num_stages+1)) != 0)
        extra_x++;
    while ((size_y + extra_y) % (1 << (*num_stages+1)) != 0)
        extra_y++;
    while (getc(fp) != '\n');    /* skip the rest */
    while (getc(fp) != '\n');
    if (type == '2') {           /* grayscale ascii */
        Y = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Y == NULL)
            throw Exception(Exception::ERR_MEMORY);
        int c;
        for (int y = 0; y < size_y; y++)
            for (int x = 0; x < size_x; x++) {
                fscanf(fp, "%d", &c);
                at(0, x, y) = (float) c;
            }
        symmetrize(0);
    } else if (type == '5') {    /* grayscale raw */
        Y = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Y == NULL)
            throw Exception(Exception::ERR_MEMORY);
        int c;         
        for (int y = 0; y < size_y; y++)
            for (int x = 0; x < size_x; x++) {
                c = fgetc(fp);
                at(0, x, y) = (float) c;
            }
        symmetrize(0);
    } else if (type == '3') {    /* color ascii */
        Y = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Y == NULL)
            throw Exception(Exception::ERR_MEMORY);
        Cb = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Cb == NULL)
            throw Exception(Exception::ERR_MEMORY);
        Cr = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Cr == NULL)
            throw Exception(Exception::ERR_MEMORY);
        int r, g, b;
        for (int y = 0; y < size_y; y++)
            for (int x = 0; x < size_x; x++) {
                fscanf(fp, "%d", &r);
                fscanf(fp, "%d", &g);
                fscanf(fp, "%d", &b);
                /* convert RGB to YCbCr */
                at(0, x, y) = 16.0 + 0.257*((float) r) + 0.504*((float) g) + 0.097*((float) b);
                at(1, x, y) = 128.0 - 0.148*((float) r) - 0.291*((float) g)  + 0.439*((float) b);
                at(2, x, y) = 128.0 + 0.439*((float) r) - 0.368*((float) g)  - 0.071*((float) b);
            }
        symmetrize(0);
        symmetrize(1);
        symmetrize(2);
    } else {                     /* color raw */
        Y = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Y == NULL)
            throw Exception(Exception::ERR_MEMORY);
        Cb = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Cb == NULL)
            throw Exception(Exception::ERR_MEMORY);
        Cr = new float[(size_x+extra_x) * (size_y+extra_y)];
        if (Cr == NULL)
            throw Exception(Exception::ERR_MEMORY);
        int r, g, b;
        for (int y = 0; y < size_y; y++)
            for (int x = 0; x < size_x; x++) {
                r = fgetc(fp);
                g = fgetc(fp);
                b = fgetc(fp);
                /* convert RGB to YCbCr */
                at(0, x, y) = 16.0 + 0.257*((float) r) + 0.504*((float) g) + 0.097*((float) b);
                at(1, x, y) = 128.0 - 0.148*((float) r) - 0.291*((float) g)  + 0.439*((float) b);
                at(2, x, y) = 128.0 + 0.439*((float) r) - 0.368*((float) g)  - 0.071*((float) b);
            }
        symmetrize(0);
        symmetrize(1);
        symmetrize(2);
    }
    fclose(fp);
}

void Image::save(char* fname) {
    FILE* fp = fopen(fname, "wb");
    if (fp == NULL)
        throw Exception(Exception::ERR_FILE_ACCESS);
    if (Cb == NULL) {            /* grayscale image? */
        fprintf(fp, "P5\n");
        fprintf(fp, "%d %d\n", size_x, size_y);
        fprintf(fp, "255\n");
        for (int y = 0; y < size_y; y++)
            for (int x = 0; x < size_x; x++) {
                float c = floor(at(0, x, y));
                if (c > 255.0) c = 255.0;
                if (c < 0.0) c = 0.0;
                fputc((int) c, fp);
            }
    } else {                     /* color image */
        fprintf(fp, "P6\n");
        fprintf(fp, "%d %d\n", size_x, size_y);
        fprintf(fp, "255\n");
        for (int y = 0; y < size_y; y++)
            for (int x = 0; x < size_x; x++) {
                /* convert YCbCr to RGB */
                float r = floor(1.164 * (at(0, x, y)-16.0) + 1.596 * (at(2, x, y) - 128.0));
                float g = floor(1.164 * (at(0, x, y)-16.0) - 0.392 * (at(1, x, y) - 128.0) - 0.813 * (at(2, x, y) - 128.0));
                float b = floor(1.164 * (at(0, x, y)-16.0) + 2.017 * (at(1, x, y) - 128.0));
                if (r > 255.0) r = 255.0;
                if (r < 0.0) r = 0.0;
                if (g > 255.0) g = 255.0;
                if (g < 0.0) g = 0.0;
                if (b > 255.0) b = 255.0;
                if (b < 0.0) b = 0.0;
                fputc((int) r, fp);
                fputc((int) g, fp);
                fputc((int) b, fp);
            }
    }
    fflush(fp);
    fclose(fp);
}

void Image::symmetrize(int component) {
    for (int y = 0; y < size_y; y++)
        for (int x = 0; x < extra_x; x++)
            at(component, size_x + x, y) = at(component, size_x - x - 1, y);
    for (int x = 0; x < size_x; x++)
        for (int y = 0; y < extra_y; y++)
            at(component, x, size_y + y) = at(component, x, size_y - y -1);
    for (int y = size_y; y < size_y + extra_y; y++)
        for (int x = size_x; x < size_x + extra_x; x++)
            at(component, x, y) = 0;
}
