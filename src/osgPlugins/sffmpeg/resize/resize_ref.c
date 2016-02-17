#include <stdint.h>

#include "resize_test.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLIP_U8(x) MIN(MAX(x, 0), 255)
#define P 1.0 / 2.2

// CONVERSION VALUES computed with analyze.py
#define A  25788
#define B  3051
#define C  7668
#define D  30353

#define DUMP_FRAME 0

void nv21_to_argb_div2_ref(int w, int h, uint32_t *dst, const uint8_t *srcY, int linesizeY, const uint8_t *srcC, int linesizeC)
{
    int x, y;
    const uint8_t *y0 = srcY;
    const uint8_t *y1 = y0 + linesizeY;
    const int linesizeY2 = linesizeY << 1;

    for (y = 0; y < h; y += 2) {
#if DUMP_FRAME
        if (y == 0) {
            for (x = 0; x < w; x ++) {
                printf("%d ", y0[x]);
            }
            printf("\n");
            for (x = 0; x < w; x ++) {
                printf("%d ", y1[x]);
            }
            printf("\n");
            for (x = 0; x < w; x ++) {
                printf("%d ", srcC[x]);
            }
            printf("\n");
            exit(0);
        }
#endif
        for (x = 0; x < w; x += 2) {
            const int y4 = y0[x + 0] + y0[x + 1] + y1[x + 0] + y1[x + 1];
            const int u = srcC[x + 0] - 128;
            const int v = srcC[x + 1] - 128;

            const uint8_t r = CLIP_U8(((y4<<12) + v * A         + (1<<13)) >> 14);
            const uint8_t g = CLIP_U8(((y4<<12) - u * B - v * C + (1<<13)) >> 14);
            const uint8_t b = CLIP_U8(((y4<<12) + u * D         + (1<<13)) >> 14);

            *dst++ = 0xffU<<24 | r<<16 | g<<8 | b;
        }
        y0 += linesizeY2;
        y1 += linesizeY2;
        srcC += linesizeC;
    }
}
