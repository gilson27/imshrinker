/************************************************/
/* Encoder.cc, (c) Rene Puchinger               */
/************************************************/

#include <cstdlib>
#include <cmath>
#include "Exception.h"
#include "FileIOStream.h"
#include "BitIOStream.h"
#include "Encoder.h"

Encoder::Encoder() {
    im1 = NULL;
    im2 = NULL;
}

Encoder::~Encoder() {
    if (im1 != NULL) delete im1;
    if (im2 != NULL) delete im2;
}

int Encoder::sub_dc(int component) {
    float dc = 0;
    for (int y = 0; y < im1->get_size_y() + im1->get_extra_y(); y++)
        for (int x = 0; x < im1->get_size_x() + im1->get_extra_x(); x++)
            dc += im1->at(component, x, y);
    dc /= (im1->get_size_x() + im1->get_extra_x()) * (im1->get_size_y() + im1->get_extra_y());
    dc = floor(dc);
    for (int y = 0; y < im1->get_size_y() + im1->get_extra_y(); y++)
        for (int x = 0; x < im1->get_size_x() + im1->get_extra_x(); x++)
            im1->at(component, x, y) -= dc;
    return (int) dc;
    
}

void Encoder::dwt_row(int component, int row, int num_items) {
    const float alpha = -1.586134342;
    const float beta = -0.05298011854;
    const float gamma = 0.8829110762;
    const float delta = 0.44355068522;
    const float xi = 1.149604398;
    
    for (int x = 0; x < num_items/2 - 1; x++)
        im2->at(component, num_items/2 + x, row) = im1->at(component, 2*x+1, row) + alpha*(im1->at(component, 2*x, row) + im1->at(component, 2*x+2, row));
    im2->at(component, num_items - 1, row) = im1->at(component, num_items-1, row) + 2*alpha*im1->at(component, num_items-2, row);
    
    im2->at(component, 0, row) = im1->at(component, 0, row) + beta * (im2->at(component, num_items/2, row) + im2->at(component, num_items/2+1,row));
    for (int x = 1; x < num_items/2; x++)
        im2->at(component, x, row) = im1->at(component, 2*x, row) + beta * (im2->at(component, num_items/2+x, row) + im2->at(component, num_items/2+x-1,row));
    
    for (int x = 0; x < num_items/2 - 1; x++)
        im2->at(component, num_items/2 + x, row) += gamma*(im2->at(component, x, row) + im2->at(component, x+1, row));
    im2->at(component, num_items - 1, row) += gamma*(im2->at(component, num_items/2 - 1, row) + im2->at(component, num_items/2-2, row));
    
    im2->at(component, 0, row) += delta*(im2->at(component, num_items/2, row) + im2->at(component, num_items/2 + 1, row));
    for (int x = 1; x < num_items/2; x++)
        im2->at(component, x, row) += delta*(im2->at(component, num_items/2+x, row) + im2->at(component, num_items/2 + x - 1, row));
    
    for (int x = 0; x < num_items/2; x++) {
        im2->at(component, x, row) *= xi;
        im2->at(component, num_items/2+x, row) /= xi;
    }    
}

void Encoder::dwt_col(int component, int col, int num_items) {
    const float alpha = -1.586134342;
    const float beta = -0.05298011854;
    const float gamma = 0.8829110762;
    const float delta = 0.44355068522;
    const float xi = 1.149604398;
    
    for (int y = 0; y < num_items/2 - 1; y++)
        im1->at(component, col, num_items/2 + y) = im2->at(component, col, 2*y+1) + alpha*(im2->at(component, col, 2*y) + im2->at(component, col, 2*y+2));
    im1->at(component, col, num_items - 1) = im2->at(component, col, num_items-1) + 2*alpha*im2->at(component, col, num_items-2);
    
    im1->at(component, col, 0) = im2->at(component, col, 0) + beta * (im1->at(component, col, num_items/2) + im1->at(component, col, num_items/2+1));
    for (int y = 1; y < num_items/2; y++)
        im1->at(component, col, y) = im2->at(component, col, 2*y) + beta * (im1->at(component, col, num_items/2+y) + im1->at(component, col, num_items/2+y-1));
    
    for (int y = 0; y < num_items/2 - 1; y++)
        im1->at(component, col, num_items/2 + y) += gamma*(im1->at(component, col, y) + im1->at(component, col, y+1));
    im1->at(component, col, num_items - 1) += gamma*(im1->at(component, col, num_items/2 - 1) + im1->at(component, col, num_items/2-2));
    
    im1->at(component, col, 0) += delta*(im1->at(component, col, num_items/2) + im1->at(component, col, num_items/2 + 1));
    for (int y = 1; y < num_items/2; y++)
        im1->at(component, col, y) += delta*(im1->at(component, col, num_items/2+y) + im1->at(component, col, num_items/2 + y - 1));
    
    for (int y = 0; y < num_items/2; y++) {
        im1->at(component, col, y) *= xi;
        im1->at(component, col, num_items/2+y) /= xi;
    }
}

void Encoder::dwt2(int component, int size_x, int size_y) {
    for (int y = 0; y < size_y; y++)
        dwt_row(component, y, size_x);
    for (int x = 0; x < size_x; x++)
        dwt_col(component, x, size_y);
}

void Encoder::dwt2full(int component) {
    int size_x = im1->get_size_x() + im1->get_extra_x();
    int size_y = im1->get_size_y() + im1->get_extra_y();
    for (int i = 0; i < num_stages; i++) {
        dwt2(component, size_x, size_y);
        size_x /= 2;
        size_y /= 2;
    }
}

void Encoder::normalize(int component) {
    for (int y = 0; y < im1->get_size_y() + im1->get_extra_y(); y++)
        for (int x = 0; x < im1->get_size_x() + im1->get_extra_x(); x++) { 
			if (im1->at(component, x, y) >= 0)
				im1->at(component, x, y) = floor(im1->at(component, x, y));
			else
				im1->at(component, x, y) = -floor(fabs(im1->at(component, x, y)));
		}
}

void Encoder::encode(char* fn_in, char* fn_out, float bit_rate) {
    im1 = new Image();
    im1->load(fn_in, &num_stages);
    im2 = new Image(im1->is_color(), im1->get_size_x(), im1->get_size_y(), im1->get_extra_x(), im1->get_extra_y());
    FileOutputStream* fout = new FileOutputStream(fn_out);
    BitOutputStream* bout = new BitOutputStream(fout);
    /* write IMS header */
    bout->put_bits('I', 8);
    bout->put_bits('M', 8);
    bout->put_bits('S', 8);
	bout->put_bits(num_stages, 6);
	bout->put_bits(im1->get_size_x(), 12);
    bout->put_bits(im1->get_size_y(), 12);
    bout->put_bits(im1->get_extra_x(), 10);
    bout->put_bits(im1->get_extra_y(), 10);
    bout->put_bit((int) im1->is_color());
	SPIHT_Encoder* spiht_enc = new SPIHT_Encoder(im1, num_stages);
    if (im1->is_color()) {
		int bits0 = (int) ceil(bit_rate * im1->get_size() * 0.6);
		int bits1 = (int) ceil(bit_rate * im1->get_size() * 0.2);
		int bits2 = (int) ceil(bit_rate * im1->get_size() * 0.2);
		bout->put_bits(bits0, 29);
		bout->put_bits(bits1, 29);
		bout->put_bits(bits2, 29);
        int dc0 = sub_dc(0);
        int dc1 = sub_dc(1);
        int dc2 = sub_dc(2);
        bout->put_bits(dc0, 8);
        bout->put_bits(dc1, 8);
        bout->put_bits(dc2, 8);
		dwt2full(0);
        dwt2full(1);
        dwt2full(2);
		normalize(0);
		normalize(1);
		normalize(2);
		spiht_enc->encode(0, bits0, bout);
		spiht_enc->encode(1, bits1, bout);
		spiht_enc->encode(2, bits2, bout);			
    } else {
		int bits0 = (int) ceil(bit_rate * im1->get_size());
		bout->put_bits(bits0, 29);
        int dc0 = sub_dc(0);
        bout->put_bits(dc0, 8);
		dwt2full(0);
		normalize(0);
		spiht_enc->encode(0, bits0, bout);
    }
	bout->flush_bits();
	bout->flush();
    delete spiht_enc;
	delete bout;
	delete fout;
}
