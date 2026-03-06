#include "sneeze-client-driver.h"

struct sneeze_driver driver;

void quit(void)
{
        (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &driver.termios[1]);
}

int main(int argument_count, char *argument_array[])
{
        struct winsize winsize;
        (void) argument_count;
        (void) argument_array;
        (void) memset(&driver, 0, sizeof driver);
        if (tcgetattr(STDIN_FILENO, &driver.termios[0]) != 0)
        {
                return EXIT_FAILURE;
        }
        atexit(quit);
        driver.termios[1] = driver.termios[0];
        driver.termios[0].c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        driver.termios[0].c_oflag &= ~(OPOST);
        driver.termios[0].c_cflag |=  (CS8);
        driver.termios[0].c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &driver.termios[0]) != 0)
        {
                return EXIT_FAILURE;
        }
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize) != 0)
        {
                return EXIT_FAILURE;
        }
        driver.screen_row = winsize.ws_row;
        driver.screen_column = winsize.ws_col + 2;
        (void) memset(driver.screen, ' ', driver.screen_column * driver.screen_row);
        {
                int i;
                char (*screen)[driver.screen_column] = (void *)driver.screen;
                for (i = 0; i < driver.screen_row; ++i)
                {
                        (void) memcpy(&screen[i][driver.screen_column - 3], "~\r\n", 3);
                }
        }

        (void) write(STDOUT_FILENO, driver.screen, driver.screen_column * driver.screen_row);
        return 0;
}

int barycentric(vector_t *a, vector_t *b, vector_t *c, vector_t *d, vector_t *e)
{
}
