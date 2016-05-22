/************************************************/
/* SPIHT.cc, (c) Rene Puchinger                 */
/************************************************/

#include <cmath>
#include "SPIHT.h"

void SPIHT_Encoder::initialize(int component, BitOutputStream* bout) {
	lip.clear();
	lsp.clear();
	lis.clear();
	int max = -999999999;
	for (int y = 0; y < im->get_size_y() + im->get_extra_y(); y++)
		for (int x = 0; x < im->get_size_x() + im->get_extra_x(); x++)
			if (std::abs(im->at(component, x, y)) > max) max = std::abs(im->at(component, x, y));
	step = (int) floor(log((float) max) / log(2.0));
	bout->put_bits(step, 8);
	for (int y = 0; y < (im->get_size_y() + im->get_extra_y()) / (1 << num_stages); y++)
		for (int x = 0; x < (im->get_size_x() + im->get_extra_x()) / (1 << num_stages); x++) {
			lip.push_back(PixItem(x, y));
			if ((x % 2 != 0) || (y % 2 != 0))
				lis.push_back(SetItem(x, y, A));
		}
}

void SPIHT_Encoder::get_successor(int x, int y, int* sx, int* sy) {
	int lx = (im->get_size_x() + im->get_extra_x()) / (1 << num_stages);
	int ly = (im->get_size_y() + im->get_extra_y()) / (1 << num_stages);
	if (x < lx && y < ly) {
		if (x % 2 == 1)
			*sx = x + lx - 1;
		else
			*sx = x;
		if (y % 2 == 1)
			*sy = y + ly - 1;
		else
			*sy = y;
		if (*sx == x && *sy == y) {
			*sx = -1;
			*sy = -1;
		}
	} else {
		*sx = 2 * x;
		*sy = 2 * y;
		if (*sx >= (im->get_size_x() + im->get_extra_x()) || *sy >= (im->get_size_y() + im->get_extra_y())) {
			*sx = -1;
			*sy = -1;
		}
	}
}

bool SPIHT_Encoder::is_significant_pixel(int component, int x, int y) {
	return (std::abs((int) im->at(component, x, y)) >= (1 << step));
}

bool SPIHT_Encoder::is_significant_set_A(int component, int x, int y, int count) {
	if (count > 1 && is_significant_pixel(component, x, y))
		return true;
	int sx, sy;
	get_successor(x, y, &sx, &sy);
	if (sx == -1 || sy == -1)
		return false;
	if (is_significant_set_A(component, sx, sy, count + 1))
		return true;
	else if (is_significant_set_A(component, sx + 1, sy, count + 1))
		return true;
	else if (is_significant_set_A(component, sx, sy + 1, count + 1))
		return true;
	else if (is_significant_set_A(component, sx + 1, sy + 1, count + 1))
		return true;
	return false;
}

bool SPIHT_Encoder::is_significant_set_B(int component, int x, int y, int count) {
	if (count > 2 && is_significant_pixel(component, x, y))
		return true;
	int sx, sy;
	get_successor(x, y, &sx, &sy);
	if (sx == -1 || sy == -1)
		return false;
	if (is_significant_set_B(component, sx, sy, count + 1))
		return true;
	else if (is_significant_set_B(component, sx + 1, sy, count + 1))
		return true;
	else if (is_significant_set_B(component, sx, sy + 1, count + 1))
		return true;
	else if (is_significant_set_B(component, sx + 1, sy + 1, count + 1))
		return true;
	return false;
}

void SPIHT_Encoder::encode(int component, int bits, BitOutputStream* bout) {
	initialize(component, bout);
	int bit_cnt = 0;
	while (step >= 0) {
		/* Sorting pass */
		/* first process LIP */
		for (int i = 0; i < lip.size(); i++) {
			bool sig = is_significant_pixel(component, lip[i].x, lip[i].y);
			bout->put_bit((int) sig);
			if (++bit_cnt > bits) return;
			if (sig) {
				lsp.push_back(PixItem(lip[i].x, lip[i].y));
				bout->put_bit(((int) im->at(component, lip[i].x, lip[i].y)) > 0 ? 0 : 1);
				if (++bit_cnt > bits) return;
				lip.erase(lip.begin() + i);
				i--;
			}
		}
		/* now process LIS */
		for (int i = 0; i < lis.size(); i++) {
			if (lis[i].type == A) {
				bool sig = is_significant_set_A(component, lis[i].x, lis[i].y);
				bout->put_bit((int) sig);
				if (++bit_cnt > bits) return;
				if (sig) {
					int sx, sy;
					get_successor(lis[i].x, lis[i].y, &sx, &sy);
					/* process the four offsprings */
					sig = is_significant_pixel(component, sx, sy);
					bout->put_bit((int) sig);
					if (++bit_cnt > bits) return;
					if (sig) {
						lsp.push_back(PixItem(sx, sy));
						bout->put_bit(((int) im->at(component, sx, sy)) > 0 ? 0 : 1);
						if (++bit_cnt > bits) return;
					} else {
						lip.push_back(PixItem(sx, sy));
					}
					sig = is_significant_pixel(component, sx + 1, sy);
					bout->put_bit((int) sig);
					if (++bit_cnt > bits) return;
					if (sig) {
						lsp.push_back(PixItem(sx + 1, sy));
						bout->put_bit(((int) im->at(component, sx + 1, sy)) > 0 ? 0 : 1);
						if (++bit_cnt > bits) return;
					} else {
						lip.push_back(PixItem(sx + 1, sy));
					}
					sig = is_significant_pixel(component, sx, sy + 1);
					bout->put_bit((int) sig);
					if (++bit_cnt > bits) return;
					if (sig) {
						lsp.push_back(PixItem(sx, sy + 1));
						bout->put_bit(((int) im->at(component, sx, sy + 1)) > 0 ? 0 : 1);
						if (++bit_cnt > bits) return;
					} else {
						lip.push_back(PixItem(sx, sy + 1));
					}
					sig = is_significant_pixel(component, sx + 1, sy + 1);
					bout->put_bit((int) sig);
					if (++bit_cnt > bits) return;
					if (sig) {
						lsp.push_back(PixItem(sx + 1, sy + 1));
						bout->put_bit(((int) im->at(component, sx + 1, sy + 1)) > 0 ? 0 : 1);
						if (++bit_cnt > bits) return;
					} else {
						lip.push_back(PixItem(sx + 1, sy + 1));
					}
					/* test if L(i, j) != 0 */
					get_successor(sx, sy, &sx, &sy);
					if (sx != -1)
						lis.push_back(SetItem(lis[i].x, lis[i].y, B));
					lis.erase(lis.begin() + i);
					i--;
				}
			} else {
				bool sig = is_significant_set_B(component, lis[i].x, lis[i].y);
				bout->put_bit((int) sig);
				if (++bit_cnt > bits) return;
				if (sig) {
					int sx, sy;
					get_successor(lis[i].x, lis[i].y, &sx, &sy);
					lis.push_back(SetItem(sx, sy, A));
					lis.push_back(SetItem(sx + 1, sy, A));
					lis.push_back(SetItem(sx, sy + 1, A));
					lis.push_back(SetItem(sx + 1, sy + 1, A));
					lis.erase(lis.begin() + i);
					i--;
				}
			}
		}
		/* Refinement pass */
		for (int i = 0; i < lsp.size(); i++) {
			if (std::abs((int) im->at(component, lsp[i].x, lsp[i].y)) >= (1 << (step + 1))) {
				bout->put_bit(((((int)std::abs((int) im->at(component, lsp[i].x, lsp[i].y))) >> step)) & 1);
				if (++bit_cnt > bits) return;
			}
		}
		/* Quantization step update */
		step--;
	}
}

/*************************************************************************************************************/

void SPIHT_Decoder::initialize(int component, BitInputStream* bin) {
	lip.clear();
	lsp.clear();
	lis.clear();
	for (int y = 0; y < im->get_size_y() + im->get_extra_y(); y++)
		for (int x = 0; x < im->get_size_x() + im->get_extra_x(); x++)
			im->at(component, x, y) = 0;
	step = bin->get_bits(8);
	for (int y = 0; y < (im->get_size_y() + im->get_extra_y()) / (1 << num_stages); y++)
		for (int x = 0; x < (im->get_size_x() + im->get_extra_x()) / (1 << num_stages); x++) {
			lip.push_back(PixItem(x, y));
			if ((x % 2 != 0) || (y % 2 != 0))
				lis.push_back(SetItem(x, y, A));
		}
}

void SPIHT_Decoder::get_successor(int x, int y, int* sx, int* sy) {
	int lx = (im->get_size_x() + im->get_extra_x()) / (1 << num_stages);
	int ly = (im->get_size_y() + im->get_extra_y()) / (1 << num_stages);
	if (x < lx && y < ly) {
		if (x % 2 == 1)
			*sx = x + lx - 1;
		else
			*sx = x;
		if (y % 2 == 1)
			*sy = y + ly - 1;
		else
			*sy = y;
		if (*sx == x && *sy == y) {
			*sx = -1;
			*sy = -1;
		}
	} else {
		*sx = 2 * x;
		*sy = 2 * y;
		if (*sx >= (im->get_size_x() + im->get_extra_x()) || *sy >= (im->get_size_y() + im->get_extra_y())) {
			*sx = -1;
			*sy = -1;
		}
	}
}

void SPIHT_Decoder::decode(int component, int bits, BitInputStream* bin) {
	initialize(component, bin);
	int bit_cnt = 0;
	while (step >= 0) {
		/* Sorting pass */
		/* first process LIP */
		for (int i = 0; i < lip.size(); i++) {
			bool sig = (bool) bin->get_bit();
			if (++bit_cnt > bits) return;
			if (sig) {
				lsp.push_back(PixItem(lip[i].x, lip[i].y));
				im->at(component, lip[i].x, lip[i].y) = (float) ((((bool) bin->get_bit()) ? -1 : 1) * (1 << step));
				if (++bit_cnt > bits) return;
				lip.erase(lip.begin() + i);
				i--;
			}
		}
		/* now process LIS */
		for (int i = 0; i < lis.size(); i++) {
			if (lis[i].type == A) {
				bool sig = (bool) bin->get_bit();
				if (++bit_cnt > bits) return;
				if (sig) {
					int sx, sy;
					get_successor(lis[i].x, lis[i].y, &sx, &sy);
					/* process the four offsprings */
					sig = (bool) bin->get_bit();
					if (++bit_cnt > bits) return;
					if (sig) {
						lsp.push_back(PixItem(sx, sy));
						im->at(component, sx, sy) = (float) ((((bool) bin->get_bit()) ? -1 : 1) * (1 << step));
						if (++bit_cnt > bits) return;
					} else {
						lip.push_back(PixItem(sx, sy));
					}
					sig = (bool) bin->get_bit();
					if (++bit_cnt > bits) return;
					if (sig) {
						lsp.push_back(PixItem(sx + 1, sy));
						im->at(component, sx + 1, sy) = (float) ((((bool) bin->get_bit()) ? -1 : 1) * (1 << step));
						if (++bit_cnt > bits) return;
					} else {
						lip.push_back(PixItem(sx + 1, sy));
					}
					sig = (bool) bin->get_bit();
					if (++bit_cnt > bits) return;
					if (sig) {
						lsp.push_back(PixItem(sx, sy + 1));
						im->at(component, sx, sy + 1) = (float) ((((bool) bin->get_bit()) ? -1 : 1) * (1 << step));
						if (++bit_cnt > bits) return;
					} else {
						lip.push_back(PixItem(sx, sy + 1));
					}
					sig = (bool) bin->get_bit();
					if (++bit_cnt > bits) return;
					if (sig) {
						lsp.push_back(PixItem(sx + 1, sy + 1));
						im->at(component, sx + 1, sy + 1) = (float) ((((bool) bin->get_bit()) ? -1 : 1) * (1 << step));
						if (++bit_cnt > bits) return;
					} else {
						lip.push_back(PixItem(sx + 1, sy + 1));
					}
					/* test if L(i, j) != 0 */
					get_successor(sx, sy, &sx, &sy);
					if (sx != -1)
						lis.push_back(SetItem(lis[i].x, lis[i].y, B));
					lis.erase(lis.begin() + i);
					i--;
				}
			} else {
				bool sig = (bool) bin->get_bit();
				if (++bit_cnt > bits) return;
				if (sig) {
					int sx, sy;
					get_successor(lis[i].x, lis[i].y, &sx, &sy);
					lis.push_back(SetItem(sx, sy, A));
					lis.push_back(SetItem(sx + 1, sy, A));
					lis.push_back(SetItem(sx, sy + 1, A));
					lis.push_back(SetItem(sx + 1, sy + 1, A));
					lis.erase(lis.begin() + i);
					i--;
				}
			}
		}
		/* Refinement pass */
		for (int i = 0; i < lsp.size(); i++) {
			if (std::abs((int) im->at(component, lsp[i].x, lsp[i].y)) >= (1 << (step + 1))) {
				if ((bool) bin->get_bit()) {
					if ((int) im->at(component, lsp[i].x, lsp[i].y) >= 0)
						im->at(component, lsp[i].x, lsp[i].y) = (float) (((int) im->at(component, lsp[i].x, lsp[i].y)) | (1 << step));
					else
						im->at(component, lsp[i].x, lsp[i].y) = (float) (-(((int) std::abs((int) im->at(component, lsp[i].x, lsp[i].y))) | (1 << step)));
				} else {
					im->at(component, lsp[i].x, lsp[i].y) = (float) (((int) im->at(component, lsp[i].x, lsp[i].y)) & (~(1 << step)));
				}
				if (++bit_cnt > bits) return;
			}
		}
		/* Quantization step update */
		step--;
	}
}
