/************************************************/
/* SPIHT.h, (c) Rene Puchinger                  */
/*                                              */
/* SPIHT encoder / decoder                      */
/************************************************/

#ifndef SPIHT_H
#define SPIHT_H

#include <vector>
#include "Image.h"
#include "BitIOStream.h"

typedef struct PixItem {
	int x;
	int y;
	PixItem(int _x, int _y) { x = _x; y = _y; }
} PixItem;

typedef enum { A, B } SetType;

typedef struct SetItem {
	int x;
	int y;
	SetType type;
	SetItem(int _x, int _y, SetType _type) { x = _x; y = _y; type = _type; }
} SetItem;

typedef std::vector<PixItem> LIP;
typedef std::vector<PixItem> LSP;
typedef std::vector<SetItem> LIS;

class SPIHT_Encoder {
	Image* im;
	int num_stages;
	LIP lip;
	LSP lsp;
	LIS lis;
	int step;    /* quantization step */
	void get_successor(int x, int y, int* sx, int* sy);
	bool is_significant_pixel(int component, int x, int y);
	bool is_significant_set_A(int component, int x, int y, int count = 1);
	bool is_significant_set_B(int component, int x, int y, int count = 1);
	void initialize(int component, BitOutputStream* bout);
public:
	SPIHT_Encoder(Image* _im, int _num_stages) { im = _im; num_stages = _num_stages; }
	void encode(int component, int bits, BitOutputStream* bout);
};

class SPIHT_Decoder {
	Image* im;
	int num_stages;
	LIP lip;
	LSP lsp;
	LIS lis;
	int step;    /* quantization step */
	void get_successor(int x, int y, int* sx, int* sy);
	void initialize(int component, BitInputStream* bin);
public:
	SPIHT_Decoder(Image* _im, int _num_stages) { im = _im; num_stages = _num_stages; }
	void decode(int component, int bits, BitInputStream* bin);
};

#endif
