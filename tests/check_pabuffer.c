#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "pabuffer.h"
#include "utime.h"

typedef struct tstruct
{
    char    str[1000];
    int     len;
} tstruct;

START_TEST(test_pabuffer_init)
{
	uint32_t size = PABUFFER_DEFAULT_EXT_COUNT;
    pabuffer *pabuf = pabuffer_init(sizeof(tstruct));
    ck_assert(pabuf != NULL);
	ck_assert(pabuf->capacity == PABUFFER_DEFAULT_EXT_COUNT);
	ck_assert(pabuf->remain_count == size);
}
END_TEST

START_TEST(test_pabuffer_get)
{
	uint32_t size = PABUFFER_DEFAULT_EXT_COUNT;

    pabuffer *pabuf = pabuffer_init(sizeof(tstruct));

	tstruct *obj = (tstruct*) pabuffer_get(pabuf);

	ck_assert(obj != NULL);

	size--;

	ck_assert(pabuf->capacity == PABUFFER_DEFAULT_EXT_COUNT);
	ck_assert(pabuf->remain_count == size);

	uint32_t i, tsize;
	for(i = 0, tsize = size; i < size; i++)
	{
		obj = (tstruct*) pabuffer_get(pabuf);
		tsize--;
		ck_assert(pabuf->capacity == PABUFFER_DEFAULT_EXT_COUNT);
		ck_assert(pabuf->remain_count == tsize);
	}
}
END_TEST

START_TEST(test_pabuffer_get_return_back)
{
	uint32_t size = PABUFFER_DEFAULT_EXT_COUNT;

    pabuffer *pabuf = pabuffer_init(sizeof(tstruct));

	tstruct *obj = (tstruct*) pabuffer_get(pabuf);
	size--;
	pabuffer_return_back(pabuf, obj);
	size++;
	tstruct *obj2  = pabuffer_get(pabuf);
	size--;
	ck_assert(obj == obj2);

	ck_assert(pabuf->remain_count == size);
}
END_TEST

START_TEST(test_pabuffer_extend)
{
	uint32_t size = PABUFFER_DEFAULT_EXT_COUNT;

    pabuffer *pabuf = pabuffer_init(sizeof(tstruct));

	uint32_t i;
	for(i = 0; i < size; i++)
	{
		pabuffer_get(pabuf);
	}

	ck_assert(pabuf->remain_count == 0);

	pabuffer_get(pabuf);

	ck_assert(pabuf->remain_count == (size-1));
}
END_TEST

START_TEST(test_pabuffer_max_remain)
{
	uint32_t max_remain = 1000;

    pabuffer *pabuf = pabuffer_init2(sizeof(tstruct), max_remain);

	printf("Consumed memory - %zd\n", pabuf->memory_size);
	printf("Colculated memory - %" PRIu64 "\n", sizeof(tstruct)*pabuf->capacity+sizeof(pabuffer_shell)*pabuf->capacity+sizeof(pabuffer));

	uint32_t i;
	for(i = 0; i < (max_remain * 10); i++)
	{
		pabuffer_get(pabuf);
	}

	printf("Consumed memory - %zd\n", pabuf->memory_size);
	printf("Colculated memory - %" PRIu64 "\n", sizeof(tstruct)*pabuf->capacity+sizeof(pabuffer_shell)*pabuf->capacity+sizeof(pabuffer));

	ck_assert(pabuf->remain_count == 0);

	pabuffer_get(pabuf);

	ck_assert(pabuf->remain_count == (PABUFFER_DEFAULT_EXT_COUNT-1));

	tstruct *obj;

	for(i = 0; i < (max_remain + (max_remain/2)); i++)
	{
		obj = calloc(sizeof(tstruct), 1);
		pabuffer_return_back(pabuf, obj);
	}

	printf("Consumed memory - %zd\n", pabuf->memory_size);
	printf("Colculated memory - %" PRIu64 "\n", (sizeof(tstruct) + sizeof(pabuffer_shell)) * pabuf->capacity + sizeof(pabuffer));

	ck_assert(pabuf->remain_count == max_remain);
}
END_TEST

START_TEST(test_performance)
{
	uint64_t timer_calloc, diff_calloc;
	uint64_t timer_pabuffer, diff_pabuffer;

	int i, count = 1000 * 1000 * 100;

	tstruct *t;

	timer_calloc = utime_time();
	for(i = 0; i < count; i++)
	{
		t = calloc(sizeof(tstruct), 1);
		free(t);

		if((i % 1000000) == 0)
		{
			printf(".");
			fflush(stdout);
		}

	}
	printf("\n");

	diff_calloc = utime_time() - timer_calloc;

	printf("Speed calloc elapesed %" PRIu64 " usec for %d rounds \n", diff_calloc, count);

    pabuffer *pabuf = pabuffer_init(sizeof(tstruct));

	timer_pabuffer = utime_time();
	for(i = 0; i < count; i++)
	{
		t  = (tstruct*) pabuffer_get(pabuf);
		pabuffer_return_back(pabuf, t);

		if((i % 1000000) == 0)
		{
			printf(".");
			fflush(stdout);
		}
	}
	printf("\n");
	diff_pabuffer = utime_time() - timer_pabuffer;
	printf("Speed pabuffer elapesed %" PRIu64 " usec for %d rounds \n", diff_pabuffer, count);
	printf("Consumed memory - %zd\n", pabuf->memory_size);
	ck_assert(diff_pabuffer < diff_calloc);
	pabuffer_free(pabuf);
}
END_TEST

Suite* pabuffer_suite(void)
{
    Suite *s = suite_create("pabuffer test suite");

    TCase *tc = tcase_create("pabuffer test case");
    tcase_add_test(tc, test_pabuffer_init);
    tcase_add_test(tc, test_pabuffer_get);
    tcase_add_test(tc, test_pabuffer_get_return_back);
    tcase_add_test(tc, test_pabuffer_extend);
    tcase_add_test(tc, test_pabuffer_max_remain);
	tcase_set_timeout(tc, 100);
    tcase_add_test(tc, test_performance);

    suite_add_tcase(s, tc);

    return s;
}

int main(void)
{
    Suite *s = pabuffer_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    return 0;
}
