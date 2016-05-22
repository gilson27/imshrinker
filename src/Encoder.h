/************************************************/
/* Encoder.h, (c) Rene Puchinger                */
/*                                              */
/* wavelet image encoder                        */
/************************************************/

#ifndef ENCODER_H
#define	ENCODER_H

#include "Image.h"
#include "SPIHT.h"

class Encoder {
	int num_stages;
    Image* im1;
    Image* im2;
    int sub_dc(int component);                                 /* subtract the (round of) dc component and return it */
    void dwt_row(int component, int row, int num_items);       /* 1D dwt on rows */
    void dwt_col(int component, int col, int num_items);       /* 1D dwt on columns */
    void dwt2(int component, int size_x, int size_y);          /* 2D dwt */
    void dwt2full(int component);                              /* 2D full dwt */
	void normalize(int component);
public:
    Encoder();
    ~Encoder();
    void encode(char* fn_in, char* fn_out, float bit_rate);
};

#endif
