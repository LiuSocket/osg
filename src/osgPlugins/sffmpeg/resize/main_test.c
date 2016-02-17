#include <stdint.h>

#include "resize_test.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLIP_U8(x) MIN(MAX(x, 0), 255)
#define P 1.0 / 2.2


// GOOD VALUES
#define A  25788
#define B  3051
#define C  7668
#define D  30353

//.short 25772, 3064, 7619, 30376
// BAD VALUES
//#define A 22970
//#define B 5638
//#define C 11700
//#define D 29032

#include "data.h"


int main(int argc, char* argv[])
{
    int x;

    //    printf("%d\n", sizeof(cr) / sizeof(int));

    for (x = 0; x < 1920; x += 2) {
        int y0 = y[x];
        const int v = cb[x / 2] - 128;
        const int u = cr[x / 2] - 128;

        const int r = CLIP_U8(((y0<<14) + v * A              + (1<<13)) >> 14);
        const int g = CLIP_U8(((y0<<14) - u *  B - v * C  + (1<<13)) >> 14);
        const int b = CLIP_U8(((y0<<14) + u * D             + (1<<13)) >> 14);

        //printf("%d, %d\n", bgra[x * 4 + 2], r);
        //printf("%d, %d\n", bgra[x * 4 + 1], g);
        printf("%d, %d\n", bgra[x * 4], b);
    }
}
