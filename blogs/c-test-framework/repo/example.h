
int fibonacci(int n) {
    int num1 = 0, num2 = 1, nextNum;
    for (int i = 1; i <= n; i++) {
        nextNum = num1 + num2;
        num1 = num2;
        num2 = nextNum;
    }
    return num1;
}