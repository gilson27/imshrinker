/************************************************/
/* Decoder.cc, (c) Rene Puchinger               */
/************************************************/

#include <cstdlib>
#include <cmath>
#include "FileIOStream.h"
#include "BitIOStream.h"
#include "Exception.h"
#include "Decoder.h"

#include <fstream>

Decoder::Decoder() {
    im1 = NULL;
    im2 = NULL;
}

Decoder::~Decoder() {
    if (im1 != NULL) delete im1;
    if (im2 != NULL) delete im2;
}

void Decoder::add_dc(int component, int dc) {
    for (int y = 0; y < im1->get_size_y() + im1->get_extra_y(); y++)
        for (int x = 0; x < im1->get_size_x() + im1->get_extra_x(); x++) {
            im1->at(component, x, y) += dc;
            im1->at(component, x, y) = floor(im1->at(component, x, y));
            if (im1->at(component, x, y) < 0) im1->at(component, x, y) = 0;       /* fix possible underflow and overflow */
            if (im1->at(component, x, y) > 255) im1->at(component, x, y) = 255;
        }
}

void Decoder::idwt_row(int component, int row, int num_items) {
    const float alpha = -1.586134342;
    const float beta = -0.05298011854;
    const float gamma = 0.8829110762;
    const float delta = 0.44355068522;
    const float xi = 1.149604398;

    for (int x = 0; x < num_items/2; x++) {
        im2->at(component, x, row) /= xi;
        im2->at(component, num_items/2+x, row) *= xi;
    }
    
    for (int x = 1; x < num_items/2; x++)
        im2->at(component, x, row) -= delta*(im2->at(component, num_items/2+x, row) + im2->at(component, num_items/2 + x - 1, row));
    im2->at(component, 0, row) -= delta*(im2->at(component, num_items/2, row) + im2->at(component, num_items/2 + 1, row));
    
    im2->at(component, num_items - 1, row) -= gamma*(im2->at(component, num_items/2 - 1, row) + im2->at(component, num_items/2-2, row));
    for (int x = 0; x < num_items/2 - 1; x++)
        im2->at(component, num_items/2 + x, row) -= gamma*(im2->at(component, x, row) + im2->at(component, x+1, row));
    
    for (int x = 1; x < num_items/2; x++)
        im1->at(component, 2*x, row) = im2->at(component, x, row) - beta * (im2->at(component, num_items/2+x, row) + im2->at(component, num_items/2+x-1,row));
    im1->at(component, 0, row) = im2->at(component, 0, row) - beta * (im2->at(component, num_items/2, row) + im2->at(component, num_items/2+1,row));

    im1->at(component, num_items-1, row) = im2->at(component, num_items - 1, row) - 2*alpha*im1->at(component, num_items-2, row);
    for (int x = 0; x < num_items/2 - 1; x++)
        im1->at(component, 2*x+1, row) = im2->at(component, num_items/2 + x, row) - alpha*(im1->at(component, 2*x, row) + im1->at(component, 2*x+2, row));    
}

void Decoder::idwt_col(int component, int col, int num_items) {
    const float alpha = -1.586134342;
    const float beta = -0.05298011854;
    const float gamma = 0.8829110762;
    const float delta = 0.44355068522;
    const float xi = 1.149604398;

    for (int y = 0; y < num_items/2; y++) {
        im1->at(component, col, y) /= xi;
        im1->at(component, col, num_items/2+y) *= xi;
    }

    for (int y = 1; y < num_items/2; y++)
        im1->at(component, col, y) -= delta*(im1->at(component, col, num_items/2+y) + im1->at(component, col, num_items/2 + y - 1));
    im1->at(component, col, 0) -= delta*(im1->at(component, col, num_items/2) + im1->at(component, col, num_items/2 + 1));

    im1->at(component, col, num_items - 1) -= gamma*(im1->at(component, col, num_items/2 - 1) + im1->at(component, col, num_items/2-2));    
    for (int y = 0; y < num_items/2 - 1; y++)
        im1->at(component, col, num_items/2 + y) -= gamma*(im1->at(component, col, y) + im1->at(component, col, y+1));
    
    for (int y = 1; y < num_items/2; y++)
        im2->at(component, col, 2*y) = im1->at(component, col, y) - beta * (im1->at(component, col, num_items/2+y) + im1->at(component, col, num_items/2+y-1));
    im2->at(component, col, 0) = im1->at(component, col, 0) - beta * (im1->at(component, col, num_items/2) + im1->at(component, col, num_items/2+1));    

    im2->at(component, col, num_items-1) = im1->at(component, col, num_items - 1) - 2*alpha*im2->at(component, col, num_items-2);    
    for (int y = 0; y < num_items/2 - 1; y++)
        im2->at(component, col, 2*y+1) = im1->at(component, col, num_items/2 + y) - alpha*(im2->at(component, col, 2*y) + im2->at(component, col, 2*y+2));    
}

void Decoder::idwt2(int component, int size_x, int size_y) {
    for (int x = 0; x < size_x; x++)
        idwt_col(component, x, size_y);
    for (int y = 0; y < size_y; y++)
        idwt_row(component, y, size_x);
}

void Decoder::idwt2full(int component, int num_stages) {
    int size_x = (im1->get_size_x() + im1->get_extra_x()) / (1 << (num_stages - 1));
    int size_y = (im1->get_size_y() + im1->get_extra_y()) / (1 << (num_stages - 1));
    for (int i = 0; i < num_stages; i++) {
        idwt2(component, size_x, size_y);
        size_x *= 2;
        size_y *= 2;
    }
}

void Decoder::decode(char* fn_in, char* fn_out) {
    FileInputStream* fin = new FileInputStream(fn_in);
    BitInputStream* bin = new BitInputStream(fin);
    int h1 = bin->get_bits(8);
    int h2 = bin->get_bits(8);
    int h3 = bin->get_bits(8);
    if (h1 != 'I' || h2 != 'M' || h3 != 'S')
        throw Exception(Exception::ERR_IMS_FORMAT);
    int num_stages = bin->get_bits(6);
	int size_x = bin->get_bits(12);
    int size_y = bin->get_bits(12);
    int extra_x = bin->get_bits(10);
    int extra_y = bin->get_bits(10);
    bool is_color = (bool) bin->get_bit();
    im1 = new Image(is_color, size_x, size_y, extra_x, extra_y);
    im2 = new Image(is_color, size_x, size_y, extra_x, extra_y);
    SPIHT_Decoder* spiht_dec = new SPIHT_Decoder(im1, num_stages);
    if (is_color) {
		int bits0 = bin->get_bits(29);
		int bits1 = bin->get_bits(29);
		int bits2 = bin->get_bits(29);
        int dc0 = bin->get_bits(8);
        int dc1 = bin->get_bits(8);
        int dc2 = bin->get_bits(8);
		spiht_dec->decode(0, bits0, bin);
		spiht_dec->decode(1, bits1, bin);
		spiht_dec->decode(2, bits2, bin);
        idwt2full(0, num_stages);
        idwt2full(1, num_stages);
        idwt2full(2, num_stages);
        add_dc(0, dc0);
        add_dc(1, dc1);
        add_dc(2, dc2);
    } else {
		int bits0 = bin->get_bits(29);
        int dc0 = bin->get_bits(8);
		spiht_dec->decode(0, bits0, bin);
        idwt2full(0, num_stages);
        add_dc(0, dc0);
    }
    im1->save(fn_out);
    delete spiht_dec;
	delete bin;
	delete fin;
}
