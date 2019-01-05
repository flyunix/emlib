#include "em/log.h"
#include "test.h"

#include <stdlib.h>

static const char * module = "MAIN_TETS";

int main(int argc, char **argv)
{
    if(argc < 2) {
        printf("Usage:./emlib log_level.\n");
        return 0;
    }
    
    int loglevel = atoi(argv[1]);

    em_log_set_log_level(loglevel);    

    EM_LOG(EM_LOG_INFO, "Hello, %s.", "Emlib.");
    EM_LOG(EM_LOG_INFO, "Enjoy, It!");

    return test_main();
}
