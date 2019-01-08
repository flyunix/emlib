#include "em/log.h"
#include "test.h"

#include <stdlib.h>

static const char * module = "MAIN_TETS";

int main(int argc, char **argv)
{
    if(argc < 2) {
        printf("Usage:emlib <log_level>.\n");
        return 0;
    }
    
    int loglevel = atoi(argv[1]);

    return test_main(loglevel);
}
