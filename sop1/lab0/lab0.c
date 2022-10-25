#include <stdio.h>

int main() {
    int x = 0;
    int i;
    int arr[1];
    printf("Hello, World!\n");

    for(i = 0; i < 100; ++i)
        x += i;

    arr[1] = 2;

    printf("Sum: %d\n", x);

    return 0;
}
