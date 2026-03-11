#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#ifndef USE_FLOAT
typedef double scalar_t;
typedef double vector_t;
typedef double matrix_t;
#else
typedef float scalar_t;
typedef float vector_t;
typedef float matrix_t;
#endif

#define TIMEVAL_SEC 0
#define TIMEVAL_USEC 1000

enum status
{
        SUCCESS = 0,
        FAILURE = 0 - 1
};

void screen_set(void);
int input(void);
int configure(void);
void raster_triangle(vector_t *a, vector_t *b, vector_t *c);

struct termios termios;
struct termios initial_termios;

#ifndef SCREEN_SIZE
#define SCREEN_SIZE 65536
#endif

char newline[] = "\r\n";
#define NEWLINE_SIZE (sizeof newline - 1)
char screen[SCREEN_SIZE];
int screen_size;
int screen_column;
int screen_row;

int main(int argument_count, char *argument_array[])
{
        scalar_t x = 0.0;
        vector_t a[3] = { 3.0 / 4.0, 0.0 / 4.0, 1.0 };
        vector_t b[3] = { 0.0 - 1.0 / 4.0, 4.0 / 4.0, 1.0 };
        vector_t c[3] = { 0.0 / 4.0, 0.0 / 4.0, 1.0 };
        int stop = 0;
        (void) argument_count;
        (void) argument_array;
        if (configure() != SUCCESS)
        {
                return EXIT_FAILURE;
        }
        while (stop != 1)
        {
                char character;
                character = input();
                if (character == ('q' & 0x1F))
                {
                        stop = 1;
                }

                /*
                        set cursor to row 1, column 1.
                        we may want to merge write calls by
                        appending to a buffer but i am
                        holding off for now.
                */
                vector_t aa[3];
                vector_t bb[3];
                vector_t cc[3];
                aa[0] = a[0];
                aa[1] = a[1];
                aa[2] = a[2] + 0.5 * sin(x);
                bb[0] = b[0];
                bb[1] = b[1];
                bb[2] = b[2] + 0.5 * sin(x);
                cc[0] = c[0];
                cc[1] = c[1];
                cc[2] = c[2] + 0.5 * sin(x);
                x += 0.01;
                screen_set();
                raster_triangle(aa, bb, cc);
                (void) write(STDOUT_FILENO, "\x1b[H", 3);
                (void) write(STDOUT_FILENO, screen, screen_size);
        }
        return EXIT_SUCCESS;
}

int input(void)
{
        char character;
        fd_set read_set;
        struct timeval timeval;
        timeval.tv_sec = TIMEVAL_SEC;
        timeval.tv_usec = TIMEVAL_USEC;
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        switch (select(1, &read_set, NULL, NULL, &timeval))
        {
        case FAILURE:
                return errno;
        case 0:
                return '\0';
        default:
                if (read(STDIN_FILENO, &character, 1) == FAILURE && errno != EAGAIN)
                {
                        return errno;
                }
        }
        return character;
}

void restore(void)
{
        /* show cursor */
        (void) write(STDOUT_FILENO, "\x1b[?25h", 6);
        /* disable alternate buffer */
        (void) write(STDOUT_FILENO, "\x1b[?1049l", 8);
        (void) tcsetattr(STDOUT_FILENO, TCSAFLUSH, &initial_termios);
}

void screen_set(void)
{
        int i;
        char (*screen_buffer)[screen_column] = (void *)screen;
        (void) memset(screen, ' ', screen_size);
        for (i = 0; i < screen_row; ++i)
        {
                (void) memcpy(&screen_buffer[i][screen_column - 3], "~\r\n", 3);
        }
}

/* this function doubles as a signal handler.  pass 0, NULL, NULL when calling it. */
void resize(int number, siginfo_t *info, void *context)
{
        int column;
        int row;
        int size;
        struct winsize winsize;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize) == FAILURE)
        {
                exit(EXIT_FAILURE);
        }
        column = screen_column;
        row = screen_row;
        size = screen_size;
        /*
                we add two to the terminal column because we
                append a \r\n at the end of each row so that
                we can write the buffer as is.
                make sure when "drawing" to the buffer that
                you do not write over these.
        */
        screen_column = winsize.ws_col + NEWLINE_SIZE;
        screen_row = winsize.ws_row;
        /*
                we subtract two here so that we do not write the last \r\n.
        */
        screen_size = screen_column * screen_row - NEWLINE_SIZE;
        if (screen_size > SCREEN_SIZE)
        {
                screen_column = column;
                screen_row = row;
                screen_size = size;
        }
        else
        {
                screen_set();
        }
}

int configure(void)
{
        struct sigaction signalaction = {0};
        /* hide cursor */
        (void) write(STDOUT_FILENO, "\x1b[?25l", 6);
        /* enable alternate buffer */
        (void) write(STDOUT_FILENO, "\x1b[?1049h", 8);
        if (tcgetattr(STDOUT_FILENO, &initial_termios) == FAILURE)
        {
                return errno;
        }
        (void) atexit(restore);
        termios = initial_termios;
        termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        termios.c_oflag &= ~(OPOST);
        termios.c_cflag |= (CS8);
        termios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &termios) == FAILURE)
        {
                return errno;
        }
        signalaction.sa_sigaction = &resize;
        if (sigaction(SIGWINCH, &signalaction, NULL) == FAILURE)
        {
                return errno;
        }
        resize(0, NULL, NULL);
        return SUCCESS;
}

int barycentric(a, b, c, d, e)
        vector_t *a;
        vector_t *b;
        vector_t *c;
        vector_t *d;
        vector_t *e;
{
        scalar_t f;
        scalar_t g;
        f = ((b[1] - c[1]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[1] - c[1]));
        if (fabs(f) < 1e-8)
        {
                return FAILURE;
        }
        g = 1.0 / f;
        e[0] = ((b[1] - c[1]) * (d[0] - c[0]) + (c[0] - b[0]) * (d[1] - c[1])) * g;
        e[1] = ((c[1] - a[1]) * (d[0] - c[0]) + (a[0] - c[0]) * (d[1] - c[1])) * g;
        e[2] = 1.0 - e[0] - e[1];
        return SUCCESS;
}

void raster_triangle(vector_t *a, vector_t *b, vector_t *c)
{
        char (*screen_buffer)[screen_column] = (void *)screen;
        vector_t d[2];
        vector_t e[2];
        vector_t f[2];
        scalar_t g;
        scalar_t h;
        vector_t i[2];
        vector_t j[2];
        vector_t k[2];
        int minx;
        int miny;
        int maxx;
        int maxy;
        int x;
        int y;
        g = (screen_column - NEWLINE_SIZE) / 2.0;
        h = screen_row / 2.0;
        /*
                learned this from tsoding.
        */
        i[0] = a[0] / a[2];
        i[1] = a[1] / a[2];
        j[0] = b[0] / b[2];
        j[1] = b[1] / b[2];
        k[0] = c[0] / c[2];
        k[1] = c[1] / c[2];
        d[0] = i[0] * g + g;
        d[1] = 0 - i[1] * h + h;
        e[0] = j[0] * g + g;
        e[1] = 0 - j[1] * h + h;
        f[0] = k[0] * g + g;
        f[1] = 0 - k[1] * h + h;
        /* get triangle bounds in "pixel" coordinates. */
        minx = d[0] < e[0] ? (d[0] < f[0] ? d[0] : f[0]) : (e[0] < f[0] ? e[0] : f[0]);
        miny = d[1] < e[1] ? (d[1] < f[1] ? d[1] : f[1]) : (e[1] < f[1] ? e[1] : f[1]);
        maxx = d[0] > e[0] ? (d[0] > f[0] ? d[0] : f[0]) : (e[0] > f[0] ? e[0] : f[0]);
        maxy = d[1] > e[1] ? (d[1] > f[1] ? d[1] : f[1]) : (e[1] > f[1] ? e[1] : f[1]);
        if (minx < 0) minx = 0;
        if (miny < 0) miny = 0;
        if (maxx > screen_column - NEWLINE_SIZE) maxx = screen_column - NEWLINE_SIZE;
        if (maxy > screen_row) maxy = screen_row;

        for (y = miny; y < maxy; ++y)
        {
                for (x = minx; x < maxx; ++x)
                {
                        scalar_t i[2];
                        vector_t j[3];
                        i[0] = x;
                        i[1] = y;
                        if (barycentric(d, e, f, i, j) == SUCCESS)
                        {
                                int aa = j[0] < -1e-8;
                                int bb = j[1] < -1e-8;
                                int cc = j[2] < -1e-8;
                                if (aa == bb && bb == cc)
                                {
                                        screen_buffer[y][x] = 'g';
                                }
                        }
                }
        }
}
