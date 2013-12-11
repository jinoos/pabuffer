#ifndef _PABUFFER_H_
#define _PABUFFER_H_

#include <stddef.h>
#include <stdint.h>

//
// PABuffer - Pre-Allocated Buffer
//
#define PABUFFER_DEFAULT_MAX_REMAIN_COUNT       1000*10
#define PABUFFER_DEFAULT_EXT_COUNT              100

typedef struct pabuffer_shell pabuffer_shell;
struct pabuffer_shell
{
    void                *obj;
    pabuffer_shell      *next;
};

typedef struct pabuffer pabuffer;
struct pabuffer
{
    // size of allocated objects
    uint32_t            capacity;

    // consumed memory size;
    size_t				memory_size;

    // size of a object
    size_t              obj_size;

    // max number of remain objects
    uint32_t            max_remain_count;

    // number of remain buffer objects
    uint32_t            remain_count;

    // number of object to extend when buffer is empty
    uint32_t            exp_count;

    // head of available object shell
    pabuffer_shell      *first_shell;

    // head of empty shell
    pabuffer_shell      *first_empty_shell;
};

pabuffer*   pabuffer_init(size_t obj_size);
pabuffer*   pabuffer_init2(size_t obj_size, uint32_t max_remain_count);
pabuffer*   pabuffer_init3(size_t obj_size, uint32_t max_remain_count, uint32_t exp_count);

uint32_t    pabuffer_capacity(pabuffer *pabuf);
size_t		pabuffer_mem_size(pabuffer *pabuf);

void*       pabuffer_get(pabuffer *pabuf);
void        pabuffer_return_back(pabuffer *pabuf, void *obj);
int			pabuffer_free(pabuffer *pabuf);

#endif // _PABUFFER_H_
