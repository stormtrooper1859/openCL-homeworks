#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    clock_t t1 = clock();

    int a = 1;
    for (int i = 0; i < 1000000; ++i) {
        a += rand();
    }

    printf("Hello, World!\n");

    clock_t t2 = clock();

    float stat = (float)(t2 - t1) / CLOCKS_PER_SEC;
    printf("%.3f", stat);

    return 0;
}
