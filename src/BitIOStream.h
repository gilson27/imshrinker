/************************************************/
/* BitIOStream.h, (c) Rene Puchinger            */
/*                                              */
/* bit-oriented input and output                */
/************************************************/

#ifndef BITIOSTREAM_H
#define BITIOSTREAM_H

#include "IOStream.h"

class BitInputStream {
    InputStream* in;
    int bit_count;
    int buffer;
public:
    BitInputStream(InputStream* _in);
    int get_bit();
    unsigned long get_bits(const int size);   /* read #size bits from input */
};

class BitOutputStream {
    OutputStream* out;
    int bit_count;
    int buffer;
public:
    BitOutputStream(OutputStream* _out);
    void put_bit(int but);
    void put_bits(const unsigned long bits, const int size);  /* write #size bits to output */
    void flush_bits();
    void flush() { flush_bits(); out->flush(); }
};

#endif
