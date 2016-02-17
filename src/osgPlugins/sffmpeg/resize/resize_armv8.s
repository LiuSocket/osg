.align 4

.globl nv21_to_argb_div2
.globl _nv21_to_argb_div2

yuvrgb_coeffs:
// CONVERSION VALUES computed with analyze.py
.short 25788, 3051, 7668, 30353

/***************************************************************
 * void nv21_to_argb_div2(int w, int h, uint32_t *dst,         *
 *                        const uint8_t *srcY, int linesizeY,  *
 *                        const uint8_t *srcC, int linesizeC); *
 **************************************************************/
nv21_to_argb_div2:
_nv21_to_argb_div2:
    ldr                 x12, yuvrgb_coeffs
    dup                 v0.2D, x12                                      /* v0 = coeffs */
    orn                 v4.8B, v4.8B, v4.8B                             /* default alpha (0xff) */
    movi                v5.8B, #128                                     /* for chroma */
    add                 x12, x3, x4                                     /* x12 = srcY + linesizeY (srcY2) */

    /* correct the pointers because we are processing 16 pixels at a time, and
       the width may not be a multiple of 16 */
    add                 w9, w0, #0xf
    and                 w9, w9, #0xfffffff0
    sub                 w7, w4, w9                                      /* linesizeY - read_width <- luma line skip */
    add                 w4, w7, w4
    sub                 w6, w6, w9                                      /* linesizeC - read_width <- chroma line skip */
    lsr                 w7, w0, #1
    add                 w9, w7, #0x7
    and                 w9, w9, #0xfffffff8
    sub                 w8, w9, w7                                      /*  align(width/2, 8) - width/2 */
    lsl                 w8, w8, #2                                      /* (align(width/2, 8) - width/2) * 4 <- dest line backward */

1:
    mov                 w7, w0                                          /* w7 = width */

2:
    prfm                pldl1strm, [x3,  #16]
    prfm                pldl1strm, [x12, #16]
    prfm                pldl1strm, [x5,  #16]
    prfm                pstl1strm, [x2,  #32]

    ld1                 {v2.16B}, [x3], #16                             /* v2: 1st luma line */
    ld1                 {v3.16B}, [x12], #16                            /* v3: 2nd luma line */
    ld2                 {v22.8B, v23.8B}, [x5], #16                     /* v22/v23: interleaved chroma line */

    usubl               v22.8H, v22.8B, v5.8B                           /* v22 = U - 128 */
    usubl               v23.8H, v23.8B, v5.8B                           /* v23 = V - 128 */
    uaddl               v16.8H, v2.8B,  v3.8B
    uaddl2              v17.8H, v2.16B, v3.16B
    uzp1                v2.8H, v16.8H, v16.8H
    uzp1                v3.8H, v17.8H, v17.8H
    uzp2                v16.8H, v16.8H, v16.8H
    uzp2                v17.8H, v17.8H, v17.8H
    uaddl               v16.4S, v16.4H, v2.4H
    uaddl2              v17.4S, v17.8H, v3.8H
    shl                 v16.4S, v16.4S, #12                             /* v16 =  Y4 << 12 (low) */
    shl                 v17.4S, v17.4S, #12                             /* v17 =  Y4 << 12 (high) */
    orr                 v18.16B, v16.16B, v16.16B
    orr                 v19.16B, v17.16B, v17.16B
    orr                 v20.16B, v16.16B, v16.16B
    orr                 v21.16B, v17.16B, v17.16B
    smlal               v16.4S, v23.4H, v0.H[0]                         /* (Y4 << 12) + V * 22970               (low,  red)   */
    smlal2              v17.4S, v23.8H, v0.H[0]                         /* (Y4 << 12) + V * 22970               (high, red)   */
    smlsl               v18.4S, v22.4H, v0.H[1]                         /* (Y4 << 12) - U *  5638                             */
    smlsl2              v19.4S, v22.8H, v0.H[1]                         /* (Y4 << 12) - U *  5638                             */
    smlsl               v18.4S, v23.4H, v0.H[2]                         /* (Y4 << 12) - U *  5638 - V * 11700   (low,  green) */
    smlsl2              v19.4S, v23.8H, v0.H[2]                         /* (Y4 << 12) - U *  5638 - V * 11700   (high, green) */
    smlal               v20.4S, v22.4H, v0.H[3]                         /* (Y4 << 12) + U * 29032               (low,  blue)  */
    smlal2              v21.4S, v22.8H, v0.H[3]                         /* (Y4 << 12) + U * 29032               (high, blue)  */
    sqrshrun            v3.4H, v16.4S, #14
    sqrshrun2           v3.8H, v17.4S, #14                              /* v1 = ({v16,v17 } + (1<<13)) >> 14 (red) */
    sqrshrun            v2.4H, v18.4S, #14
    sqrshrun2           v2.8H, v19.4S, #14                              /* v2 = ({v18,v19} + (1<<13)) >> 14 (green) */
    sqrshrun            v1.4H, v20.4S, #14
    sqrshrun2           v1.8H, v21.4S, #14                              /* v3 = ({v20,v21} + (1<<13)) >> 14 (blue) */
    uqxtn               v1.8B, v1.8H                                    /* red   8b */
    uqxtn               v2.8B, v2.8H                                    /* green 8b */
    uqxtn               v3.8B, v3.8H                                    /* blue  8b */
    st4                 {v1.8B, v2.8B, v3.8B, v4.8B}, [x2], #32         /* save R,G,B,A */

    subs                w7, w7, #16
    b.gt                2b

    sub                 x2, x2, x8                                      /* dst   line backward adjustement */
    add                 x3, x3, x4                                      /* srcY  line forward  adjustement */
    add                 x12, x12, x4                                    /* srcY2 line forward  adjustement */
    add                 x5, x5, x6                                      /* srcC  line forward  adjustement */

    subs                w1, w1, #2
    b.gt                1b

    ret
