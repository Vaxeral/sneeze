#include "sneeze-client-driver.h"

struct sneeze_driver driver;

int main()
{
        float x = 0.0;
        /* triangle */
        vector_t a[3] = { 3.0 / 8.0, 0.0, 0.0 };
        vector_t b[3] = { (0.0 - 1.0) / 8.0, 4.0 / 8.0, 0.0 };
        vector_t c[3] = { 0.0, 0.0, 0.0 };

        if (driver_initialize(&driver) != 0)
        {
                return EXIT_FAILURE;
        }
        while (1)
        {
                int status;
                char character;
                fd_set readfds;
                struct timeval timeval;
                FD_ZERO(&readfds);
                FD_SET(STDIN_FILENO, &readfds);
                timeval.tv_sec = 0;
                timeval.tv_usec = 10000; /* microseconds. */
                status = select(1, &readfds, NULL, NULL, &timeval);
                switch (status)
                {
                case 0 - 1:
                        return EXIT_FAILURE;
                case 0:
                        character = '\0';
                        break;
                default:
                        if (read(STDIN_FILENO, &character, 1) == -1 && errno != EAGAIN)
                        {
                                return EXIT_FAILURE;
                        }
                }
                if (character == ('q' & 0x1f))
                {
                        break;
                }
                driver_screen_clear(&driver);
                /* graphics */
                x += 0.01;
                vector_t aa[3];
                vector_t bb[3];
                vector_t cc[3];

                aa[0] = a[0] + 0.5 * cos(x);
                aa[1] = a[1] + 0.5 * sin(x);
                aa[2] = a[2];

                bb[0] = b[0] + 0.5 * cos(x);
                bb[1] = b[1] + 0.5 * sin(x);
                bb[2] = b[2];

                cc[0] = c[0] + 0.5 * cos(x);
                cc[1] = c[1] + 0.5 * sin(x);
                cc[2] = c[2];

                // driver_screen_draw_triangle(&driver, aa, bb, cc);
                driver_screen_draw_triangle(&driver, aa, bb, cc);

                write(STDOUT_FILENO, "\x1b[H", 3);
                write(STDOUT_FILENO, driver.screen, driver.screen_size);
        }
        return 0;
}

void quit(void)
{
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &driver.termios[1]);
}

int driver_screen_draw_triangle(struct sneeze_driver *driver, vector_t *a, vector_t *b, vector_t *c)
{
        int ii;
        int jj;
        vector_t d[3];
        if (triangle_normal(a, b, c, d) != 0)
        {
                return 0 - 1;
        }
        for (ii = 0; ii < driver->screen_row; ++ii)
        {
                for (jj = 0; jj < driver->screen_column - 2; ++jj)
                {
                        vector_t e[3];
                        vector_t f[3];
                        scalar_t g;
                        vector_t h[3];
                        vector_t i[3];
                        char (*screen)[driver->screen_column] = (void *)driver->screen;
                        e[0] = 0.0;
                        e[1] = 0.0;
                        e[2] = 0.0 - 1.0;
                        f[0] = (scalar_t)jj / driver->screen_column * 2.0 - 1.0;
                        f[1] = -((scalar_t)ii / driver->screen_row * 2.0 - 1.0);
                        f[2] = 1.0;
                        if (plane_hit(a, d, e, f, &g) != 0)
                        {
                                continue;
                        }
                        h[0] = e[0] + f[0] * g;
                        h[1] = e[1] + f[1] * g;
                        h[2] = e[2] + f[2] * g;
                        if (barycentric(a, b, c, h, i) != 0)
                        {
                                continue;
                        }
                        /* check if barycentric coordinates are within 0.0 and 1.0 this means the point is within the triangle and the triangle should be colored. */
                        if (i[0] <= 1.0 && i[0] >= 0e-8 && i[1] <= 1.0 && i[1] >= 0e-8 && i[0] + i[1] <= 1.0)
                        {
                                screen[ii][jj] = '.';
                        }
                }
        }
        return 0;
}

int driver_screen_clear(struct sneeze_driver *driver)
{
        memset(driver->screen, ' ', driver->screen_size);
        {
                int i;
                char (*screen)[driver->screen_column] = (void *)driver->screen;
                for (i = 0; i < driver->screen_row; ++i)
                {
                        memcpy(&screen[i][driver->screen_column - 2], "\r\n", 2);
                }
        }
        return 0;
}

int driver_initialize(struct sneeze_driver *driver)
{
        struct winsize winsize;
        memset(driver, 0, sizeof *driver);
        if (tcgetattr(STDIN_FILENO, &driver->termios[0]) != 0)
        {
                exit(EXIT_FAILURE);
        }
        atexit(quit);
        driver->termios[1] = driver->termios[0];
        driver->termios[0].c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        driver->termios[0].c_oflag &= ~(OPOST);
        driver->termios[0].c_cflag |= (CS8);
        driver->termios[0].c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &driver->termios[0]) != 0)
        {
                exit(EXIT_FAILURE);
        }
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize) != 0)
        {
                exit(EXIT_FAILURE);
        }
        driver->screen_column = winsize.ws_col + 2;
        driver->screen_row = winsize.ws_row;
        driver->screen_size = driver->screen_column * driver->screen_row - 2;
        driver_screen_clear(driver);
        return 0;
}

int barycentric(vector_t *a, vector_t *b, vector_t *c, vector_t *d, vector_t *e)
{
        vector_t f[3];
        vector_t g[3];
        vector_t h[3];
        scalar_t i;
        scalar_t j;
        scalar_t k;
        scalar_t l;
        scalar_t m;
        scalar_t n;
        f[0] = b[0] - a[0];
        f[1] = b[1] - a[1];
        f[2] = b[2] - a[2];
        g[0] = c[0] - a[0];
        g[1] = c[1] - a[1];
        g[2] = c[2] - a[2];
        h[0] = d[0] - a[0];
        h[1] = d[1] - a[1];
        h[2] = d[2] - a[2];
        i = f[0] * f[0] + f[1] * f[1] + f[2] * f[2];
        j = f[0] * g[0] + f[1] * g[1] + f[2] * g[2];
        k = g[0] * g[0] + g[1] * g[1] + g[2] * g[2];
        l = h[0] * f[0] + h[1] * f[1] + h[2] * f[2];
        m = h[0] * g[0] + h[1] * g[1] + h[2] * g[2];
        n = i * k - j * j;
        if (fabs(n) <= 0e-8)
        {
                return 0 - 1;
        }
        e[0] = (k * l - j * m) / n;
        e[1] = (i * m - j * l) / n;
        e[2] = 1.0 - e[0] - e[1];
        return 0;
}

int plane_hit(vector_t *a, vector_t *b, vector_t *c, vector_t *d, scalar_t *e)
{
        scalar_t f;
        vector_t g[3];
        scalar_t h;
        f = b[0] * d[0] + b[1] * d[1] + b[2] * d[2];
        if (fabs(f) <= 0e-8)
        {
                return 0 - 1;
        }
        g[0] = a[0] - c[0];
        g[1] = a[1] - c[1];
        g[2] = a[2] - c[2];
        h = (g[0] * b[0] + g[1] * b[1] + g[2] * b[2]) / f;
        if (h < 0e-8)
        {
                return 0 - 1;
        }
        *e = h;
        return 0;
}

int triangle_normal(vector_t *a, vector_t *b, vector_t *c, vector_t *d)
{
        vector_t e[3];
        vector_t f[3];
        vector_t g[3];
        scalar_t h;
        e[0] = a[0] - c[0];
        e[1] = a[1] - c[1];
        e[2] = a[2] - c[2];
        f[0] = b[0] - c[0];
        f[1] = b[1] - c[1];
        f[2] = b[2] - c[2];
        g[0] = e[1] * f[2] - e[2] * f[1];
        g[1] = e[2] * f[0] - e[0] * f[2];
        g[2] = e[0] * f[1] - e[1] * f[0];
        h = sqrt(g[0] * g[0] + g[1] * g[1] + g[2] * g[2]);
        if (fabs(h) <= 0e-8)
        {
                return 0 - 1;
        }
        d[0] = g[0] / h;
        d[1] = g[1] / h;
        d[2] = g[2] / h;
        return 0;
}
