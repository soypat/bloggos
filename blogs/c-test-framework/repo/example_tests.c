#include "testing.h"
#include "example.h"

TEST(TestFibonacci)
void TestFibonacci(testing_T *t) {
    int cases[] = {0,1,1,2,3,5,8,13};
    for (int i = 0;i < sizeof(cases)/sizeof(i); i++) {
        int want = cases[i];
        int got = fibonacci(i);
        TASSERTF(t, got==want, "got fibo(%d)=%d, want %d",i,got,want);
    }
}