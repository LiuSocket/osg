#include <stdio.h>
#include <stdlib.h>

#include "resize_test.h"

int main(int ac, char **av)
{
    if (ac != 3) {
        fprintf(stderr, "Usage: %s <width> <height>\n", av[0]);
        return 0;
    }
    if (resize_test(atoi(av[1]), atoi(av[2])) == 0) {
        printf("OK\n");
        return 0;
    }
    return 1;
}
