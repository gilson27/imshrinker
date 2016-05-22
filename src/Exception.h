/************************************************/
/* Exception.h, (c) Rene Puchinger              */
/*                                              */
/* a simple exception class                     */
/************************************************/

#ifndef EXCEPTION_H
#define EXCEPTION_H

class Exception {
public:
    enum ERR_CODE {
        ERR_MEMORY,
        ERR_FILE_NOT_FOUND,
        ERR_FILE_ACCESS,
        ERR_FILE_WRITE,
        ERR_IMAGE_FORMAT,
        ERR_IMS_FORMAT
    };
private:
    ERR_CODE e;
public:
    Exception(enum ERR_CODE _e): e(_e) {};
    char const* get_message();
};

#endif
