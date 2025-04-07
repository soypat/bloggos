# Building a C test framework in 10 minutes

It has come to my attention C programmers are now looking to switch over to memory safe languages
just because the government made some blog post on *safety*.

Don't jump ship on C just yet, you can **test** your code. 
- 100% of issues in C code are missed because a failing test for the case is not written.
- Memory safety can be solved by testing your code

People will often go on about how Rust enables developers to write safe code you would not be able to write in C.
But they don't talk about **tests**.

Testing C code is not hard. One starts by writing a testing framework from scratch.
Here's what user's test code could look like for our framework for a test of a function that returns the nth fibonacci number:

```c
int cases[] = {0,1,1,2,3,5,8,13};
for (int i = 0;i < sizeof(cases)/sizeof(i); i++) {
    int want = cases[i];
    int got = fibonacci(i);
    ASSERTF(got==want, "got fibo(%d)=%d, want %d", i, got, want);
}
```
Pretty neat. We write our tests in C so we feel comfortable writing tests since we already know C and want to call our tested code easily.

Since we are writing C we need to write our code inside a function.
```c
void TestFibonacci(testing_T *t)
```
Our function is called on a `testing_T` type which will contain meta information about our test and also results of test (failed/errors/fatal error).

```c
// T is a state passed to each test which is modified by the test itself.
// Stores whether test failed.
typedef struct {
    char *name;
    bool failed;
} testing_T;
```

So this results in the following program:
```c
void main() {
    testing_T t;
    t.failed=false;
    TestFibonacci(&t);
    if (t.failed) {
        printf("test failed");
    } else {
        printf("test passed");
    }
}
```

Yeah, that's nice, we can test our code but we are lacking in the user experience department. Users will have to
- Write tests and add them manually to `main` program
- Put all test and program logic within same file
- Test coverage? Get out of here

We can do a lot better by imposing some rules on how tests are written
1. Thou shalt write tests in test files, indicated by "_tests.c" prefix
2. Thou shalt declare the test logic at compile time

The second rule is a pretty big constraint. We want users to declare the test logic but not actually do any calling to register the test during runtime. By having the test definitions be purely static it gives users some pretty big guarantees on when the tests run and leaves the door open for future setup/teardown piplines if need be.

To implement rule 2 we define a macro that users use to specify which functions are test functions to be added to the test corpus:
```c
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
```

The use of `__attribute__` might be confusing- we are specifying that a function is to be run at initialization of the program, before `main` is run. This allows us to "magically" register all tested functions before the tests are run in `main`. This requires the user to call `TEST` macro on the test function name. So our resulting `fibonacci_tests.c` file ends up looking like:

```c
#include "testing.h"
#include "fibonacci.h"

TEST(TestFibonacci)
void TestFibonacci(testing_T *t) {
    int cases[] = {0,1,1,2,3,5,8,13};
    for (int i = 0;i < sizeof(cases)/sizeof(i); i++) {
        int want = cases[i];
        int got = fibonacci(i);
        TASSERTF(t, got==want, "got fibo(%d)=%d, want %d",i,got,want);
    }
}
```

where `TASSERTF` is just a fancy `if` statement that sets `t->failed=1` and prints the message if the condition is false. This macro could potentially return too.

This is pretty much all the user needs to know. To add a new test the user must:
- Add a new function in a file that ends with `_tests.c` suffix with the signature `void testname(testing_T *t)` including the `testing.h` header.
- Add the function to the test corpus by calling the `TEST` macro on the function name. Conventionally the macro is called in the line above the function declaration.

That's it:
```sh
1/1 tests passed
✅ PASS with 100% test coverage
```

### Wait, when did you count the tests and check coverage?

The `testing` library contains this code I have not shown you yet.
```c
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
```

And this is the `main` program. All it does really is call `testing_run_tests` defined above and pretty print the results.
```c
#include "testing.h"
#include <stdio.h>

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"

int main() {
    testing_results results;
    int exit = testing_run_tests(&results);
    int totalrun = results.failed+results.passed;
    bool failed = results.failed > 0;
    char* color = failed ? RED : GREEN;
    printf("%s%d/%d\033[0m tests passed\n", color, results.passed, results.num_tests);
    if (failed) {
        printf("⛔ FAIL\n");
    } else {
        printf("✅ PASS\n");
    }
    return exit;
}
```
OK, so maybe I lied. We are still missing the code coverage and the makefile used. Cmake 3.6 is an excellent choice. We set the `--coverage` flag and link with `gcov` for that tasty coverage.

```python
# This file defines the build for the tests in this project.
# using the proposed test framework.
cmake_minimum_required(VERSION 3.6) # 3.6 required for regex filtering.

project(framework_example C)

# C99 is best C. Add coverage support.
add_definitions(-std=c99 --coverage)

# Output the executables and objects to the current directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

include_directories(PUBLIC .)

# test_sources will contain all build source code.
file(GLOB_RECURSE test_sources "*.c")
# Filter out any unwanted source files that are not part of your project
list(FILTER test_sources EXCLUDE REGEX "CMakeFiles")

# Build the test executable.
add_executable(tests.bin ${test_sources})

# Link the gcov library for coverage instrumentation support.
# Required to get test coverage.
target_link_libraries(tests.bin gcov)
```

To calculate the coverage build the project and use lcov:

```
$ cmake . && make && ./tests.bin
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /home/pato/Documents/src/personal/bloggos/blogs/c-test-framework/repo
[ 25%] Building C object CMakeFiles/tests.bin.dir/run_tests.c.o
[ 50%] Linking C executable tests.bin
[100%] Built target tests.bin
1/1 tests passed
✅ PASS
```

```
$ lcov -q -c -d . -o coverage.info --exclude "/usr*" --exclude "*tests.c*" --exclude "*testing.c*" --exclude "*testing.h"
$ lcov --summary coverage.info | awk '/lines/ {print $2}'
100.0%
```
Voilá. You now have a framework for testing C code with code coverage enabled.