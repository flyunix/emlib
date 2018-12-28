#include "em/log.h"
#include "test.h"

int main(void)
{
    em_log_set_log_level(EM_LOG_DEBUG);    

    EM_LOG(EM_LOG_DEBUG, "Hello, %s.", "Emlib.");
    EM_LOG(EM_LOG_DEBUG, "Enjoy, It!");

    test_main();
}
