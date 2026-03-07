#ifndef SNEEZE_CLIENT_DRIVER_H_
#define SNEEZE_CLIENT_DRIVER_H_

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

typedef double scalar_t;
typedef double vector_t;
typedef double matrix_t;

int triangle_normal(vector_t *a, vector_t *b, vector_t *c, vector_t *d);
int plane_hit(vector_t *a, vector_t *b, vector_t *c, vector_t *d, scalar_t *e);
int barycentric(vector_t *a, vector_t *b, vector_t *c, vector_t *d, vector_t *e);

#ifndef SCREEN_SIZE
#define SCREEN_SIZE 65536
#endif

struct sneeze_driver
{
        struct termios termios[2];
        char screen[SCREEN_SIZE];
        int screen_column;
        int screen_row;
        int screen_size;
};

int driver_initialize(struct sneeze_driver *driver);
int driver_screen_clear(struct sneeze_driver *driver);
int driver_screen_draw_triangle(struct sneeze_driver *driver, vector_t *a, vector_t *b, vector_t *c);

#endif
