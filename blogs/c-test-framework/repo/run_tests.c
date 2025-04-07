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