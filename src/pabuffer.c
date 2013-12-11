#include <stdlib.h>

#include "pabuffer.h"

int pabuffer_extend(pabuffer* pabuf, size_t obj_size, uint32_t size);
pabuffer_shell* pabuffer_alloc(size_t obj_size);

pabuffer*   pabuffer_init(size_t obj_size)
{
	return pabuffer_init3(obj_size, PABUFFER_DEFAULT_MAX_REMAIN_COUNT, PABUFFER_DEFAULT_EXT_COUNT);
}

pabuffer*   pabuffer_init2(size_t obj_size, uint32_t max_remain_count)
{
	return pabuffer_init3(obj_size, max_remain_count, PABUFFER_DEFAULT_EXT_COUNT);
}


pabuffer*   pabuffer_init3(size_t obj_size, uint32_t max_remain_count, uint32_t exp_count)
{
	int ret;

    pabuffer *pabuf = calloc(sizeof(pabuffer), 1);
    if(pabuf == NULL)
        return NULL;

    pabuf->obj_size = obj_size;
    pabuf->memory_size += sizeof(pabuffer);
    pabuf->max_remain_count = max_remain_count;
    pabuf->exp_count = exp_count;

    ret = pabuffer_extend(pabuf, obj_size, exp_count);

    if(ret == 0)
    {
    	free(pabuf);
    	return NULL;
    }

    return pabuf;
}

int pabuffer_extend(pabuffer* pabuf, size_t obj_size, uint32_t size)
{
    int i;
    pabuffer_shell *shell;
    shell = NULL;
    for(i = 0; i < size; i++)
    {
        shell = pabuffer_alloc(pabuf->obj_size);

        if(shell == NULL)
            break;

        shell->next = pabuf->first_shell;
        pabuf->first_shell = shell;
        pabuf->capacity++;
        pabuf->remain_count++;
        pabuf->memory_size += (obj_size + sizeof(pabuffer_shell));
    }

    return i;
}

pabuffer_shell* pabuffer_alloc(size_t obj_size)
{
    pabuffer_shell *shell = calloc(sizeof(*shell), 1);
    if(shell == NULL)
        return NULL;

    shell->obj = calloc(obj_size, 1);
    if(shell->obj == NULL)
    {
        free(shell);
        return NULL;
    }

    return shell;
}

uint32_t pabuffer_capacity(pabuffer *pabuf)
{
	return pabuf->capacity;
}

// @brief Get size of memory 
uint64_t pabuffer_mem_size(pabuffer *pabuf)
{
	return pabuf->memory_size;
}

// @brief Obtain a object to use, you must cast to proper object
// @param Pabuffer pointer
// @return object, it must be casted to proper object
void* pabuffer_get(pabuffer *pabuf)
{
	if(pabuf->first_shell == NULL)
	{
		if(pabuffer_extend(pabuf, pabuf->obj_size, pabuf->exp_count) == 0)
		{
			return NULL;
		}
	}

	pabuffer_shell *shell = pabuf->first_shell;
	pabuf->first_shell = shell->next;
	shell->next = pabuf->first_empty_shell;
	pabuf->first_empty_shell = shell;
	pabuf->remain_count--;
	return shell->obj;
}

// @bfief Return back object into pabuffer
// @param Pabuffer pointer
// @param Object which is finished to use
void pabuffer_return_back(pabuffer *pabuf, void *obj)
{
	pabuffer_shell *shell = pabuf->first_empty_shell;
	pabuf->first_empty_shell = shell->next;
	if(pabuf->max_remain_count == pabuf->remain_count)
	{
		free(shell);
		free(obj);
		pabuf->memory_size -= (sizeof(pabuffer_shell)+pabuf->obj_size);
		pabuf->capacity--;
	}else
	{
		shell->obj = obj;
		shell->next = pabuf->first_shell;
		pabuf->first_shell = shell;
		pabuf->remain_count++;
	}
	return;
}

// CAUTION Don't call pabuffer_free before return all buffer objects
//          back into pabuffer. If not, will be passible to occurred
//          memory leak or sagmant-fault.
// 
// @brief Release all allocated memory (if passible)
// @param Pabuffer pointer
// @returns
//		0 = if pabuffer is null
//		0 < number of released objects
int pabuffer_free(pabuffer *pabuf)
{
	if(pabuf == NULL)
		return 0;

	void *obj;
	pabuffer_shell *shell;
	uint32_t count;

	while(pabuf->first_shell != NULL)
	{
		shell = pabuf->first_shell;
		pabuf->first_shell = shell->next;
		obj = shell->obj;
		free(obj);
		free(shell);
		count++;
	}

	while(pabuf->first_empty_shell != NULL)
	{
		shell = pabuf->first_empty_shell;
		pabuf->first_empty_shell = shell->next;
		free(shell);
	}

	free(pabuf);

	return count;
}
