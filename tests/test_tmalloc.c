#include <stdio.h>
#include <check.h>
#include "../include/tmalloc.h"

START_TEST(simple_check)
{
    int *arr = (int *)tmalloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++)
    {
        arr[i] = i;
    }
    for (int i = 0; i < 10; i++)
    {
        ck_assert_int_eq(arr[i], i);
    }

    tfree(arr);
}
END_TEST

// void alloc_freed()
// {
//     int *arr = (int *)tmalloc(sizeof(int) * 10);

//     tfree(arr);
//     arr = (int *)tmalloc(sizeof(int) * 10);
// }

// Create the test suite
Suite *test_suite(void)
{
    Suite *suite;
    TCase *test_case;

    suite = suite_create("TestSuite");

    // Create a test case and add it to the suite
    test_case = tcase_create("SimpleCheck");
    tcase_add_test(test_case, simple_check);
    suite_add_tcase(suite, test_case);

    return suite;
}

int main()
{
    int num_failed;
    Suite *suite;
    SRunner *runner;

    suite = test_suite();
    runner = srunner_create(suite);

    // Run the test suite
    srunner_run_all(runner, CK_NORMAL);
    num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}