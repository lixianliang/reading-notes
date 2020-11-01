#include <stdio.h>

int main() {
    unsigned int x = 1233444;
    int y = -1;

    if (y > x) {
        printf("no y > x\n");
    } else {
        printf("ok y < x\n");
    }

    return 0;
}
