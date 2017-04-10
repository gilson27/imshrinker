/************************************************/
/* Image.cc, (c) Rene Puchinger                 */
/************************************************/

#include <iostream>
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

// reads An entire frame and pass it to split to three channels
void read_frame(FILE* in, int frame_number, yuv_frame *vid_frame, yuv_video *vid_params)
{
    char* fbuff;
    int read_length;

    fbuff = (char *)malloc(vid_params->frame_size);
    long offset = frame_number * vid_params->frame_size;
    fseek(in, offset, SEEK_SET);
    printf("file size %d\n", vid_params->frame_size);
    read_length = fread(fbuff, 1, vid_params->frame_size, in);
    fprintf(stdout, "Read %d bytes.\n", read_length);
    read_yuv(fbuff, vid_frame, vid_params);
    return 0;
}

// reads YUV File and returns three channels
void read_yuv(char* fbuff, yuv_frame *vid_frame, yuv_video *vid_params)
{
    int word = 0;
    int yindex = 0, cbindex = 0, crindex = 0;
    int x, y;
    int i, j;
    int xpitch, ypitch, chxpitch, chypitch;
    xpitch = 1 << wv_log2i(vid_params->width - 1);
    ypitch = 1 << wv_log2i(vid_params->height - 1);
    chxpitch = 1 << wv_log2i(vid_params->chwidth - 1);
    chypitch = 1 << wv_log2i(vid_params->chheight - 1);

    printf("xpitch = %d ypitch = %d\n", xpitch, ypitch);

    vid_frame->Y  = malloc(xpitch * ypitch * sizeof(vid_frame->Y));
    vid_frame->Cb = malloc(chxpitch * chypitch * sizeof(vid_frame->Cb));
    vid_frame->Cr = malloc(chxpitch * chypitch * sizeof(vid_frame->Cr));

    printf("In read YUV \n");
    /**
     * Read The buffer in YUV v210 format
     * YUV v210 4:2:0 format
     *   |**8bit**|**8bit**|**8bit**|**8bit**|
     * 1 |00VVVVVV|VVVVYYYY|YYYYYYUU|UUUUUUUU|
     * 2 |00YYYYYY|YYYYUUUU|UUUUUUYY|YYYYYYYY|
     * 3 |00UUUUUU|UUUUYYYY|YYYYYYVV|VVVVVVVV|
     * 4 |00YYYYYY|YYYYVVVV|VVVVVVYY|YYYYYYYY|
     */
    for(y = 0; y < vid_params->height; y++)
    {

        /**
            Read Four + Four words
        */
        for(x = 0; x < vid_params->vwidth; x+=8)
        {
            /**
                Set byte swapped integer for reading word
            */
            word = 0;
            //printf("row = %x\n", fbuff[x + 3]);
            word = word | ((int)fbuff[(y*vid_params->vwidth) + x + 3] & 0xFF) << 24 | ((int)fbuff[(y*vid_params->vwidth) + x + 2] & 0xFF) << 16 | ((int)fbuff[(y*vid_params->vwidth) + x + 1] & 0xFF) << 8 | ((int)fbuff[(y*vid_params->vwidth) + x] & 0xFF);
            if(x == 16 && y == 0) {
                printf("%x\n", word);
            }
            vid_frame->Cb[cbindex] = ((word & (int)0x000003FF));
            vid_frame->Y[yindex] = (word & (int)0x000FFC00) >> 10;
            vid_frame->Cr[crindex] = (word & (int)0x3FF00000) >> 20;
            cbindex++;
            crindex++;
            yindex++;
            /**
                Set byte swapped integer for reading word
            */
            word = 0;
            word = word | ((int)fbuff[(y*vid_params->vwidth) + x + 7] & 0xFF) << 24 | ((int)fbuff[(y*vid_params->vwidth) + x + 6] & 0xFF) << 16 | ((int)fbuff[(y*vid_params->vwidth) + x + 5] & 0xFF) << 8 | ((int)fbuff[(y*vid_params->vwidth) + x + 4] & 0xFF);
            if(x == 16 && y == 0) {
                printf("%x\n", word);
            }
            vid_frame->Y[yindex] = word & (int)0x000003FF;
            yindex++;
            vid_frame->Cb[cbindex] = (word & (int)0x000FFC00) >> 10;
            vid_frame->Y[yindex] = (word & (int)0x3FF00000) >> 20;
            cbindex++;
            yindex++;

            /** Second set */
            x += 8;
            /**
                Set byte swapped integer for reading word
            */
            word = 0;
            word = word | ((int)fbuff[(y*vid_params->vwidth) + x + 3] & 0xFF) << 24 | ((int)fbuff[(y*vid_params->vwidth) + x + 2] & 0xFF) << 16 | ((int)fbuff[(y*vid_params->vwidth) + x + 1] & 0xFF) << 8 | ((int)fbuff[(y*vid_params->vwidth) + x] & 0xFF);
            vid_frame->Cr[crindex] = word & (int)0x000003FF;
            vid_frame->Y[yindex] = (word & (int)0x000FFC00) >> 10;
            vid_frame->Cb[cbindex] = (word & (int)0x3FF00000) >> 20;
            cbindex++;
            crindex++;
            yindex++;

            /**
                Set byte swapped integer for reading word
            */
            word = 0;
            word = word | ((int)fbuff[(y*vid_params->vwidth) + x + 7] & 0xFF) << 24 | ((int)fbuff[(y*vid_params->vwidth) + x + 6] & 0xFF) << 16 | ((int)fbuff[(y*vid_params->vwidth) + x + 5] & 0xFF) << 8 | ((int)fbuff[(y*vid_params->vwidth) + x + 4] & 0xFF);
            vid_frame->Y[yindex] = word & (int)0x000003FF;
            yindex++;
            vid_frame->Cr[crindex] = (word & (int)0x000FFC00) >> 10;
            vid_frame->Y[yindex] = (word & (int)0x3FF00000) >> 20;
            crindex++;
            yindex++;
        }
        yindex+=(xpitch - vid_params->width);
        cbindex+=(chxpitch - vid_params->chwidth);
        crindex+=(chxpitch - vid_params->chwidth);
    }

    /**
        Fill the extra space
    */
    for (i = 0; i < vid_params->height; i++)
        for (j = vid_params->width; j < xpitch; j++)
            vid_frame->Y[i * xpitch + j] = vid_frame->Y[i * xpitch + vid_params->width - 1];
    for (i = vid_params->height; i < ypitch; i++)
        memcpy(vid_frame->Y + i * xpitch, vid_frame->Y + (vid_params->height - 1) * xpitch, xpitch * sizeof *vid_frame->Y);

    for (i = 0; i < vid_params->chheight; i++)
        for (j = vid_params->chwidth; j < chxpitch; j++)
            vid_frame->Cr[i * chxpitch + j] = vid_frame->Cr[i * chxpitch + vid_params->width - 1];
    for (i = vid_params->chheight; i < chypitch; i++)
        memcpy(vid_frame->Cr + i * chxpitch, vid_frame->Cr + (vid_params->chheight - 1) * chxpitch, chxpitch * sizeof *vid_frame->Cr);

    for (i = 0; i < vid_params->chheight; i++)
        for (j = vid_params->chwidth; j < chxpitch; j++)
            vid_frame->Cb[i * chxpitch + j] = vid_frame->Cb[i * chxpitch + vid_params->width - 1];
    for (i = vid_params->chheight; i < chypitch; i++)
        memcpy(vid_frame->Cb + i * chxpitch, vid_frame->Cb + (vid_params->chheight - 1) * chxpitch, chxpitch * sizeof *vid_frame->Cb);
    return 0;
}

/**
 * Write YUV to a file
 */
void write_frame(FILE* out, int frame_number, yuv_frame *vid_frame, yuv_video * vid_params) {
    int x, y;
    int word;
    int bytes_written;
    int byte_swapped;
    int yindex = 0, crindex = 0, cbindex = 0;
    printf("In write YUV frame number %d \n", frame_number);
    /**
     * Write The buffer in YUV v210 format
     * YUV v210 4:2:0 format
     *   |**8bit**|**8bit**|**8bit**|**8bit**|
     * 1 |00VVVVVV|VVVVYYYY|YYYYYYUU|UUUUUUUU|
     * 2 |00YYYYYY|YYYYUUUU|UUUUUUYY|YYYYYYYY|
     * 3 |00UUUUUU|UUUUYYYY|YYYYYYVV|VVVVVVVV|
     * 4 |00YYYYYY|YYYYVVVV|VVVVVVYY|YYYYYYYY|
     */

    if(vid_frame->Y == NULL) 
    {
        printf("%s\n", "Empty or Null Y buffer, Unable to write to file.");
        return -1;
    }
    if(vid_frame->Cr == NULL) 
    {
        printf("%s\n", "Empty or Null Cb buffer, Unable to write to file.");
        return -1;
    }
    if(vid_frame->Cb == NULL) 
    {
        printf("%s\n", "Empty or Null Cr buffer, Unable to write to file.");
        return -1;
    }

    for(y = 0; y < vid_params->height; ++y) {
        for(x = 0; x < vid_params->width; x+=6) {
            word = 0;
            byte_swapped = 0;

            word = word | (vid_frame->Cr[crindex] & 0x3FF) << 20 | (vid_frame->Y[yindex] & 0x3FF) << 10 | (vid_frame->Cb[cbindex] & 0x3FF) << 0 ;

            if(x == 6 && y == 0) {
                printf("%x\n", word);
            }
            bytes_written = fwrite(&word, 1, sizeof(byte_swapped), out);
            //printf("bytes written = %d\n", bytes_written);

            cbindex++;
            crindex++;
            yindex++;

            word = 0;
            byte_swapped = 0;
            word = word | (vid_frame->Y[yindex+1] & 0x3FF) << 20 | (vid_frame->Cb[cbindex] & 0x3FF) << 10 | (vid_frame->Y[yindex] & 0x3FF) << 0 ;
            if(x == 6 && y == 0) {
                printf("%x\n", word);
            }
            bytes_written = fwrite(&word, 1, sizeof(byte_swapped), out);
            //printf("bytes written = %d\n", bytes_written);

            yindex++;
            cbindex++;
            yindex++;

            word = 0;
            byte_swapped = 0;
            word = word | (vid_frame->Cb[cbindex] & 0x3FF) << 20 | (vid_frame->Y[yindex] & 0x3FF) << 10 | (vid_frame->Cr[crindex] & 0x3FF) << 0 ;
            bytes_written = fwrite(&word, 1, sizeof(byte_swapped), out);
            //printf("bytes written = %d\n", bytes_written);

            cbindex++;
            crindex++;
            yindex++;

            word = 0;
            byte_swapped = 0;
            word = word | (vid_frame->Y[yindex+1] & 0x3FF) << 20 | (vid_frame->Cr[crindex] & 0x3FF) << 10 | (vid_frame->Y[yindex] & 0x3FF) << 0 ;
            bytes_written = fwrite(&word, 1, sizeof(byte_swapped), out);
            //printf("bytes written = %d\n", bytes_written);

            yindex++;
            crindex++;
            yindex++;

        }
    }
    printf("bytes written %d\n", bytes_written);
    return 0;
}


void Image::load_v210(char* fname, int* num_stages) {
    std::cout << "-- in Load v210" << std::endl;
    /**
     * Fixed size for HD
    */
    size_x = HD_WIDTH;
    size_y = HD_HEIGHT;

    FILE* fp = fopen(fname, "rb");
    if (fp == NULL)
        throw Exception(Exception::ERR_FILE_NOT_FOUND);

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

    /**
     * Read and store v210
    */
    
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
