#ifndef SNEEZE_CLIENT_DRIVER_H_
#define SNEEZE_CLIENT_DRIVER_H_

#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

typedef double matrix_t;
typedef double vector_t;
typedef double scalar_t;

#ifndef SCREEN_BUFFER_SIZE
#define SCREEN_BUFFER_SIZE 65536
#endif

struct sneeze_driver
{
        struct termios termios[2];
        char screen[SCREEN_BUFFER_SIZE];
        int screen_row;
        int screen_column;
};

#endif
