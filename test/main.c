#include "em/log.h"
#include "test.h"

static const char * module = "MAIN_TETS";

int main(void)
{
    em_log_set_log_level(EM_LOG_TRACE);    

    EM_LOG(EM_LOG_INFO, "Hello, %s.", "Emlib.");
    EM_LOG(EM_LOG_INFO, "Enjoy, It!");

    return test_main();
}
