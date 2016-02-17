#ifndef RESIZE_H
#define RESIZE_H

#define RESIZE_DIV2_FINAL_ROOM 32

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    void nv21_to_argb_div2_ref(int w, int h, uint32_t *dst, const uint8_t *srcY, int linesizeY, const uint8_t *srcC, int linesizeC);
    void nv21_to_argb_div2(int w, int h, uint32_t *dst, const uint8_t *srcY, int linesizeY, const uint8_t *srcC, int linesizeC);
#ifdef __cplusplus
}
#endif

#endif
