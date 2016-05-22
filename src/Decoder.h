/************************************************/
/* Decoder.h, (c) Rene Puchinger                */
/*                                              */
/* wavelet image decoder                        */
/************************************************/

#ifndef DECODER_H
#define	DECODER_H

#include "Image.h"
#include "SPIHT.h"

class Decoder {
private:
    Image* im1;
    Image* im2;
    void add_dc(int component, int dc);                        /* add back the dc component */
    void idwt_row(int component, int row, int num_items);      /* 1D inverse dwt on rows */
    void idwt_col(int component, int col, int num_items);      /* 1D inverse dwt on columns */
    void idwt2(int component, int size_x, int size_y);         /* 2D inverse dwt */
    void idwt2full(int component, int num_stages);             /* 2D full inverse dwt */
    void inormalize(int component, int min, int div_factor);   /* undo wavelet coefficients normalization */
public:
    Decoder();
    ~Decoder();
    void decode(char* fn_in, char* fn_out);
};

#endif
