#include <TargetConditionals.h>

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "resize_test.h"

static int check_for_mismatch(const uint32_t *b1, const uint32_t *b2, int w, int h)
{
    int x, y, mismatch = 0;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (b1[y * w + x] != b2[y * w + x]) {
                fprintf(stderr, "mismatch at x=%d and y=%d (%08X != %08X)\n",
                        x, y, b1[y * w + x], b2[y * w + x]);
                mismatch++;
            }
        }
    }
    if (mismatch > 1)
        fprintf(stderr, "%d mismatch%s\n", mismatch, mismatch > 1 ? "es" : "");
    return mismatch;
}

static uint32_t get_rand(uint32_t *x)
{
    *x = *x * 1664525 + 1013904223;
    return *x;
}

#define RAND_BYTE() ((uint8_t)(get_rand(&rand) / (double)(1LL<<32) * 255))
#define ALIGN 32

int resize_test(int w, int h)
{
    int x, y, cw, ch;
    int linesizeY, linesizeC, dst_bufsize;
    uint8_t *srcY;
    uint8_t *srcC;
    uint32_t *dst_c;
    uint32_t *dst_neon;
    uint32_t *dst_aarch64;
    uint32_t rand = 0;

    cw = w / 2;
    ch = h / 2;
    linesizeY = w    + ALIGN;
    linesizeC = cw*2 + ALIGN;
    dst_bufsize = (w/2 * 4 + ALIGN) * (h/2 + 1);

    printf("luma   %dx%d linesize:%d\n",  w,  h, linesizeY);
    printf("chroma %dx%d linesize:%d\n", cw, ch, linesizeC);
    printf("dst    %dx%d rgba\n", w/2, h/2);

    posix_memalign((void**)&dst_c,       ALIGN, dst_bufsize);
    posix_memalign((void**)&dst_neon,    ALIGN, dst_bufsize);
    posix_memalign((void**)&dst_aarch64, ALIGN, dst_bufsize);
    posix_memalign((void**)&srcY, ALIGN, linesizeY *  h);
    posix_memalign((void**)&srcC, ALIGN, linesizeC * ch);

    /* extra poison for linesizes */
    memset(srcY, 0x12, linesizeY *  h);
    memset(srcC, 0x34, linesizeC * ch);

    /* "random" luma data */
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
            srcY[y * linesizeY + x] = RAND_BYTE();

    /* "random" chroma data */
    for (y = 0; y < ch; y++)
        for (x = 0; x < cw; x++)
            srcC[y * linesizeC + x] = RAND_BYTE();

    nv21_to_argb_div2_ref(w, h, dst_c, srcY, linesizeY, srcC, linesizeC);
#if TARGET_IPHONE_SIMULATOR
    nv21_to_argb_div2_ref(w, h, dst_c, srcY, linesizeY, srcC, linesizeC);
#else
    nv21_to_argb_div2(w, h, dst_neon, srcY, linesizeY, srcC, linesizeC);
#endif

    return check_for_mismatch(dst_c, dst_neon, w/2, h/2);
}
