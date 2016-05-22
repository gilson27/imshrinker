/************************************************/
/* BitIOStream.cpp, (c) Rene Puchinger          */
/************************************************/

#include "BitIOStream.h"

BitInputStream::BitInputStream(InputStream* _in) {
    in = _in;
    bit_count = 0;
    buffer = 0;
}

int BitInputStream::get_bit() {
    if (bit_count == 0) {
        buffer = in->get_char();
        if (buffer == EOF)
            return 0;
        else
            bit_count = 8;
    }
    return (buffer >> --bit_count) & 1;
}

unsigned long BitInputStream::get_bits(const int size) {
    unsigned long value = 0;
    for (int i = 0; i < size; i++) {
        value <<= 1;
        value |= get_bit();
    }
    return value;
}

/*****************************************************************************************/

BitOutputStream::BitOutputStream(OutputStream* _out) {
    out = _out;
    bit_count = 0;
    buffer = 0;
}

void BitOutputStream::put_bit(int bit) {
    buffer <<= 1;
    buffer |= bit;
    if (++bit_count == 8) {
        out->put_char(buffer);
        bit_count = 0;
        buffer = 0;
}
}

void BitOutputStream::put_bits(const unsigned long bits, const int size) {
    for (int i = size - 1; i >= 0; i--)
        put_bit((bits >> i) & 1);
}

void BitOutputStream::flush_bits() {
    while (bit_count > 0)
        put_bit(0);
}
