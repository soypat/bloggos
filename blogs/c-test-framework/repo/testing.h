#include <stdio.h>

typedef char bool;

// T is a state passed to each test which is modified by the test itself.
// Stores whether test failed.
typedef struct {
    char *name;
    bool failed;
} testing_T;

// TestFunc is the function signature of a test function.
typedef void (*testing_TestFunc)(testing_T* t);

typedef struct {
    // name is the name of the test as assigned by user.
    char *name;

    testing_TestFunc fn;
    bool done;
    bool failed;
} testing_Test;

typedef struct {
    int num_tests;
    int failed;
    int passed;
} testing_results;

#define _maxtests 2048
// _tests stores the tests.
extern testing_Test _tests[_maxtests];
extern int _numtests;

int testing_run_tests(testing_results *results);

// TEST adds the function with the signature void name(testing_T* t) to the global list of tests to run.
#define TEST(testname)                                                    \
	void testname(testing_T*);                                            \
	__attribute__((constructor)) static void testing_add_##testname(void) \
	{                                                                 \
        if (_numtests >=_maxtests) { printf("test limit %d exceeded", _maxtests); };                    \
		testing_Test test = {0};                                      \
        test.fn = testname;                                           \
        test.name = #testname;                                        \
        _tests[_numtests] = test;                                     \
        _numtests++;                                                  \
	}

// TASSERTF is a test assertion that takes a format string and arguments in printf fashion.
// First argument is the testing_T* t, second is the expression to test,
// third is the format string, and the rest are the arguments to the format string.
#define TASSERTF(t, cond, format, ...)                                                       \
	if (!(cond)) {                                                                           \
		t->failed = 1;                                                                       \
		printf("%s assert %s:%d: " format "\n", t->name, __FILE__, __LINE__, ##__VA_ARGS__); \
	}

