/************************************************/
/* Application.h, (c) Rene Puchinger            */
/*                                              */
/* sample wavelet image compression program     */
/************************************************/

#ifndef APPLICATION_H
#define APPLICATION_H

#include "Encoder.h"
#include "Decoder.h"

class Application {
    enum { ENCODE, DECODE, IDLE } state;
    Encoder* encoder;
    Decoder* decoder;
    char* fn_in;
    char* fn_out;
    float bit_rate;
public:
    Application(int argc, char** argv);
    ~Application();
    void run();
};

#endif
