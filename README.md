# sneeze
a client and server for the first person dungeon crawler, sneeze.

## compilation
you can run the bash script provided for both server and client or compile them yourself.
- client

`cc -O3 -o sneeze-client sneeze-common.c sneeze-client.c sneeze-client-driver.c -lm`
- server

`cc -O3 -o sneeze-server sneeze-common.c sneeze-server.c sneeze-server-driver.c -lm`
the client uses a fixed sized buffer for for its screen.  pass `-DSCREEN_SIZE=65336` for example.

## note
a hardware accelerated terminal is recommended.

## about
the client is ran from a terminal.  all output is ascii.  to see visuals at a higher resolution resize your terminal, e.g. press ctrl and minus to shrink the terminal.
