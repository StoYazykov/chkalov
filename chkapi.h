// chkapi.h
#include "chkalov.h"
typedef struct {
    void* (*alloc)(struct ChkEnv* e, size_t s);
    const cv *_pool;
    char **_heap;
    size_t *_hs, *_hp;
} ChkEnv;
