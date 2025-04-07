#include "testing.h"

// tests stores the tests.
testing_Test _tests[_maxtests];

int _numtests = 0;

int testing_run_tests(testing_results *results) {
    results->failed = 0;
    results->passed = 0;
    results->num_tests = _numtests;
    for (int i = 0; i < _numtests; i++) {
        testing_Test *test = &_tests[i];
        testing_T t = {.name=test->name, .failed=0};
        test->fn(&t); // run test.
        if (t.failed != 0) {
            results->failed++;
        } else {
            results->passed++;
        }
    }
    return 0;
}
