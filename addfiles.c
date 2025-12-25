#include <stdio.h>

int main(void) {
    int i, j, max;

    printf("max? ");
    scanf("%d", &max);

    for (i = 2; i <= max; i++) {
        for (j = 2; j < i; j++) {
            if (i % j == 0) break;
            }
        if (j == i) {
	printf("%d\t", i);
	}
    }
    return 0;
}