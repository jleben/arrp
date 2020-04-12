
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#include <vector>
#include <condition_variable>
#include <mutex>

#include "jack_client.h"

int
main (int argc, char *argv[])
{
    arrp::jack_io::Jack_Client client;

    sleep (-1);
}
