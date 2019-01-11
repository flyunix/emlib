#include "em/log.h"
#include "test.h"

#include <stdlib.h>
#include <signal.h>
#include <string.h>

void handle(int signum)
{
    printf("receive signal:%d\n", signum);
}

static const char * module = "MAIN_TETS";

int main(int argc, char **argv)
{
    if(argc < 2) {
        printf("Usage:emlib <log_level>.\n");
        return 0;
    }
    
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle;
    sigaction(SIGUSR1, &sa, NULL);

    int loglevel = atoi(argv[1]);

    return test_main(loglevel);
}
