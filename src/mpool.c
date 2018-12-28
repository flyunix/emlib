#include "em/mpool.h"

#include <stdlib.h>

EM_DEF(void*) em_pool_alloc(em_pool_t *pool, em_size_t size)
{
    return malloc(size);
}

