/************************************************/
/* Application.cc, (c) Rene Puchinger           */
/************************************************/

#include <iostream>
#include <ctime>
#include <cstdio>
#include "Application.h"

Application::Application(int argc, char** argv) {
    encoder = NULL;
    decoder = NULL;
    
    std::cout << "\nImShrinker v. 0.2 (SPIHT algorithm demonstration)\n(c) 2008 Rene Puchinger\n\n";
    
    if (argc != 4) {
        std::cout << "Usage:   imshrinker [c[bpp]|d] [file_in] [file_out]\n";
        std::cout << "Where:   c[bpp] - compress with bit rate bpp (default: bpp = 0.5)\n";
        std::cout << "         d      - decompress\n";
        std::cout << "Example: imshrinker c0.1 image.ppm image.ims\n";
        std::cout << "         imshrinker d image.ims image.ppm\n";
        state = IDLE;
    } else {
        if (argv[1][0] == 'c') {
	        if (argv[1][1] != 0)
				sscanf(argv[1], "c%f", &bit_rate);
            else
                bit_rate = 0.5;
            encoder = new Encoder;
            state = ENCODE;
            fn_in = argv[2];
            fn_out = argv[3];
        } else if (argv[1][0] == 'd') {
            decoder = new Decoder;
            state = DECODE;
            fn_in = argv[2];
            fn_out = argv[3];
        } else {
			std::cout << "Usage:   imshrinker [c[bpp]|d] [file_in] [file_out]\n";
			std::cout << "Where:   c[bpp] - compress with bit rate bpp (default: bpp = 0.5)\n";
			std::cout << "         d      - decompress\n";
			std::cout << "Example: imshrinker c0.1 image.ppm image.ims\n";
			std::cout << "         imshrinker d image.ims image.ppm\n";
            state = IDLE;
        }
    }
}

Application::~Application() {
    if (encoder)
        delete encoder;
    if (decoder)
        delete decoder;
}

void Application::run() {
    clock_t start = clock();
    if (state == ENCODE) {
        std::cout << "Compressing with bit rate " << bit_rate << " ...\n";
        encoder->encode(fn_in, fn_out, bit_rate);
        std::cout << "Done! ";
        std::cout << "Time elapsed: " << (float) (clock() - start) / CLOCKS_PER_SEC << " seconds.\n";
    } else if (state == DECODE) {
        std::cout << "Decompressing ...\n";
        decoder->decode(fn_in, fn_out);
        std::cout << "Done! ";
        std::cout << "Time elapsed: " << (float) (clock() - start) / CLOCKS_PER_SEC << " seconds.\n";
    }
}
