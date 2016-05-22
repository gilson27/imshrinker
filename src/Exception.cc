/************************************************/
/* Exception.cc, (c) Rene Puchinger             */
/************************************************/

#include "Exception.h"

char const* Exception::get_message() {
    switch (e) {
        case ERR_MEMORY: return "Not enough memory.";
        case ERR_FILE_NOT_FOUND: return "File not found.";
        case ERR_FILE_ACCESS: return "File access denied.";
        case ERR_FILE_WRITE: return "File write error.";
        case ERR_IMAGE_FORMAT: return "Invalid input file format (must be a PGM or PPM).";
        case ERR_IMS_FORMAT: return "Invalid input file format (must be a valid IMS).";
    }
    return "An exception occured!";
}
