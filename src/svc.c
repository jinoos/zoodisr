#include "svc.h"

struct svc* svc_alloc(const char *name, size_t name_len, int8_t key)
{
    struct svc *svc = calloc(sizeof(struct svc), 1);

    rcounter_init(&svc->counter);
    
    svc->name = sdsnewlen(name, name_len);
    svc->key = key;
    return svc;
}

